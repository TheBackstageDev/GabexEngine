#include "GWRenderer.hpp"
#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWRenderer::GWRenderer(GWindow &window, GWinDevice &device) : window(window), GDevice(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    GWRenderer::~GWRenderer()
    {
        freeCommandBuffers();
    }
    
    void GWRenderer::createCommandBuffers()
    {
        commandBuffers.resize(GWinSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = GDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(GDevice.device(), &allocInfo, commandBuffers.data()) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void GWRenderer::recreateSwapChain()
    {
        auto extent = window.getExtent();

        while(extent.height == 0 || extent.width == 0)
        {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(GDevice.device());
        if(swapChain == nullptr)
        {
            swapChain = std::make_shared<GWinSwapChain>(GDevice, extent);
        } else {
            std::shared_ptr<GWinSwapChain> oldSwapChain = std::move(swapChain);
            swapChain = std::make_shared<GWinSwapChain>(GDevice, extent, oldSwapChain);
    
            if(!oldSwapChain->compareSwapChainFormats(*swapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format has changed!");
            }

            if(swapChain->imageCount() != commandBuffers.size())
            {
                freeCommandBuffers();
                createCommandBuffers();
            }
        }

        window.frameBufferResizedFlagReset();
    }

        void GWRenderer::freeCommandBuffers()
        {
            vkFreeCommandBuffers(
                GDevice.device(),
                GDevice.getCommandPool(),
                static_cast<uint32_t>(commandBuffers.size()),
                commandBuffers.data());
            commandBuffers.clear();
        }

        VkCommandBuffer GWRenderer::startFrame()
        {
            assert(!hasFrameStarted && "Cannot call startFrame is frame has alreadly began!");
            auto result = swapChain->acquireNextImage(&currentImageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR || window.hasWindowBeenResized())
            {
                recreateSwapChain();
                return nullptr;
            }

            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }

            hasFrameStarted = true;

            auto commandBuffer = getCurrentCommandBuffer();

            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            return commandBuffer;
        }

        void GWRenderer::endFrame()
        {
            assert(hasFrameStarted && "Cannot call endFrame when frame not in progress!");
            auto commandBuffer = getCurrentCommandBuffer();

            if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }

            auto result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || window.hasWindowBeenResized())
            {
                window.frameBufferResizedFlagReset();
                recreateSwapChain();
            }

            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }

            hasFrameStarted = false;
            currentFrameIndex = (currentFrameIndex + 1) % GWinSwapChain::MAX_FRAMES_IN_FLIGHT;
        }

        void GWRenderer::startSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {
            assert(hasFrameStarted && "Can't call startSwapChainRenderPass when frame not in progress!");
            assert(commandBuffer == getCurrentCommandBuffer() && "Cannot startSwapChainRenderPass on commandBuffer from Different frame!");

            VkRenderingAttachmentInfoKHR colorAttachment{};
            colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            colorAttachment.imageView = swapChain->getImageView(currentImageIndex);
            colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            VkClearValue clearColor = {{{0.01f, 0.0f, 0.0f, 1.0f}}};
            colorAttachment.clearValue = clearColor;

            VkRenderingAttachmentInfoKHR depthAttachment{};
            depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
            depthAttachment.imageView = swapChain->getDepthImageView(currentImageIndex);
            depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            VkClearValue depthClear = {{1.0f, 0}};
            depthAttachment.clearValue = depthClear;

            VkRenderingInfoKHR renderingInfo{};
            renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
            renderingInfo.renderArea.offset = {0, 0};
            renderingInfo.renderArea.extent = swapChain->getSwapChainExtent();
            renderingInfo.layerCount = 1;
            renderingInfo.colorAttachmentCount = 1;
            renderingInfo.pColorAttachments = &colorAttachment;
            renderingInfo.pDepthAttachment = &depthAttachment;

            vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
        }

        void GWRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {
            assert(hasFrameStarted && "Can't call endSwapChainRenderPass when frame not in progress!");
            assert(commandBuffer == getCurrentCommandBuffer() && "Cannot endSwapChainRenderPass on commandBuffer from Different frame!");

            vkCmdEndRenderingKHR(commandBuffer);

            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.image = swapChain->getCurrentImage(currentImageIndex);
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; 
            barrier.dstAccessMask = 0;

            vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,          
                0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
        }
}

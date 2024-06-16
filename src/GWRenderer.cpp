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
            commandBuffers.resize(swapChain->imageCount());

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
            swapChain = std::make_unique<GWinSwapChain>(GDevice, extent);

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
        }

        void GWRenderer::startSwapChainRenderPass(VkCommandBuffer commandBuffer)
        {
            assert(hasFrameStarted && "Can't call startSwapChainRenderPass when frame not in progress!");
            assert(commandBuffer == getCurrentCommandBuffer() && "Cannot startSwapChainRenderPass on commandBuffer from Different frame!");

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = swapChain->getRenderPass();
            renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

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

            vkCmdEndRenderPass(commandBuffer);
        }
    }
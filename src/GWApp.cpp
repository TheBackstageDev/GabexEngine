#include "GWApp.hpp"
#include <stdexcept>
#include <array>
#include <iostream>

namespace GWIN
{
    GWapp::GWapp()
    {
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    GWapp::~GWapp()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void GWapp::run()
    {
        int num;
        while (!GWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
        }

        vkDeviceWaitIdle(GDevice.device());
    }

    void GWapp::createPipelineLayout()
    {
        ;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(GDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Create Pipeline Layout!");
        }
    }

    void GWapp::createPipeline()
    {
        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(
            pipelineConfig,
            swapChain.width(),
            swapChain.height());
        pipelineConfig.renderPass = swapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        Pipeline = std::make_unique<GPipeLine>(
            GDevice,
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.vert.spv",
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.frag.spv",
            pipelineConfig);
    }

    void GWapp::createCommandBuffers()
    {
        commandBuffers.resize(swapChain.imageCount());

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

        for (int i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = swapChain.getRenderPass();
            renderPassInfo.framebuffer = swapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            Pipeline->bind(commandBuffers[i]);
            vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void GWapp::drawFrame()
    {
        uint32_t imageIndex;
        auto result = swapChain.acquireNextImage(&imageIndex);
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        result = swapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
    }
}
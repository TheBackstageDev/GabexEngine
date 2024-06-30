#pragma once

#include "../GWindow.hpp"
#include "../GWSwapChain.hpp"

//std
#include <array>
#include <memory>
#include <vector>
#include <cassert>


namespace GWIN
{
    class GWRenderer
    {
    public:
        GWRenderer(GWindow& window, GWinDevice& device);
        ~GWRenderer();
        
        GWRenderer(const GWRenderer &) = delete;
        GWRenderer &operator=(const GWRenderer &) = delete;

        VkRenderPass getRenderPass() const { return swapChain->getRenderPass(); }
        float getAspectRatio() const { return swapChain->extentAspectRatio(); }
        float getImageCount() const { return swapChain->imageCount(); }

        bool hasFrameBegan() const { return hasFrameStarted; };

        VkCommandBuffer getCurrentCommandBuffer() const { 
            assert(hasFrameStarted && "Cannot get commandBuffer when frame not in Progress!");
            return commandBuffers[currentFrameIndex];
        };

        int getFrameIndex() const
        {
            assert(hasFrameStarted && "Cannot get frameIndex when frame not in Progress!");
            return currentFrameIndex;
        }

        VkCommandBuffer startFrame();
        void endFrame();
        void startSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

        VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
        void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();

        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<GWinSwapChain> swapChain;
        GWindow& window;
        GWinDevice &GDevice;
        
        uint32_t currentImageIndex{0};
        int currentFrameIndex{0};
        bool hasFrameStarted{false};
    };
}
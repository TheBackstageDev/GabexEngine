#pragma once

#include "GWDevice.hpp"

// vulkan headers
#include <volk/volk.h>

// std lib headers
#include <string>
#include <vector>
#include <memory>

namespace GWIN
{
    class GWinSwapChain
    {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

        GWinSwapChain(GWinDevice &deviceRef, VkExtent2D windowExtent);
        GWinSwapChain(GWinDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<GWinSwapChain> previous);
        ~GWinSwapChain();

        GWinSwapChain(const GWinSwapChain &) = delete;
        void operator=(const GWinSwapChain &) = delete;
        
        void init();
        VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return renderPass; }
        VkImageView getImageView(int index) { return swapChainImageViews[index]; }
        VkImage getCurrentImage(int index) { return swapChainImages[index]; }
        VkImageView getDepthImageView(int index) { return depthImageViews[index]; }
        size_t imageCount() { return swapChainImages.size(); }
        static VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
        static VkFormat getSwapChainDepthFormat() { return swapChainDepthFormat; }
        VkExtent2D getSwapChainExtent() { return swapChainExtent; }
        uint32_t width() { return swapChainExtent.width; }
        uint32_t height() { return swapChainExtent.height; }

        float extentAspectRatio()
        {
            return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
        }
        VkFormat findDepthFormat();

        VkResult acquireNextImage(uint32_t *imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

        bool compareSwapChainFormats(const GWinSwapChain& swapChain) const
        {
            return swapChainImageFormat == swapChain.swapChainImageFormat && swapChainDepthFormat == swapChain.swapChainDepthFormat;
        }

    private:
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        void createRenderPass();
        void createFramebuffers();
        void createSyncObjects();

        // Helper functions
        VkSurfaceFormatKHR
        chooseSwapSurfaceFormat(
            const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(
            const std::vector<VkPresentModeKHR> &availablePresentModes);
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        static VkFormat swapChainImageFormat;
        static VkFormat swapChainDepthFormat;
        VkExtent2D swapChainExtent;

        std::vector<VkFramebuffer> swapChainFramebuffers;
        VkRenderPass renderPass;

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImageAllocations;
        std::vector<VkImageView> depthImageViews;
        std::vector<VkImage> swapChainImages;
        std::vector<VkImageView> swapChainImageViews;

        GWinDevice &device;
        VkExtent2D windowExtent;
        VkCommandPool commandPool;
        VkSwapchainKHR swapChain;
        std::shared_ptr<GWinSwapChain> oldSwapChain;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkSemaphore> computeFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        std::vector<VkFence> computeInFlightFences;
        size_t currentFrame = 0;
    };

} // namespace GWIN
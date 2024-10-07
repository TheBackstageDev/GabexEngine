#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"

// std
#include <array>
#include <memory>
#include <vector>
#include <cassert>

namespace GWIN
{
    class GWShadowRenderer
    {
    public:
        GWShadowRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount);
        ~GWShadowRenderer();

        VkImage getCurrentImage() const { return depthImages[imageIndex]; }
        VkImageView getCurrentImageView() const { return depthImageViews[imageIndex]; }

        void startOffscreenRenderPass(VkCommandBuffer commandBuffer);
        void endOffscreenRenderPass(VkCommandBuffer commandBuffer);

        void createNextImage();

        VkSampler getImageSampler() { return imageSampler; }

    private:
        GWindow &window;
        GWinDevice &device;

        void init(float imageCount);
        void createImageSampler();

        void createDepthResources(float imageCount);

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;
        
        VkFormat &depthFormat;

        VkSampler imageSampler;

        uint32_t imageIndex{0};
    };
}
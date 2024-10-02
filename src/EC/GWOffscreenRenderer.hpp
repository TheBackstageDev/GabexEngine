#pragma once

#include "../GWDevice.hpp"
#include "GWRendererToolkit.hpp"

// std
#include <array>
#include <memory>
#include <vector>
#include <cassert>

namespace GWIN
{
    class GWOffscreenRenderer
    {
    public:
        GWOffscreenRenderer(GWindow &window, GWinDevice &device, size_t imageCount, VkFormat depthFormat, VkFormat colorFormat);
        ~GWOffscreenRenderer();

        VkImage getCurrentImage() const { return images[imageIndex];}
        VkImageView getCurrentImageView() const { return imageViews[imageIndex]; }

        void startOffscreenRenderPass(VkCommandBuffer commandBuffer);
        void endOffscreenRenderPass(VkCommandBuffer commandBuffer);

        void createNextImage();

        VkSampler getImageSampler() { return imageSampler; }
    private:
        GWindow &window;
        GWinDevice &device;

        void init(size_t imageCount);
        void createImageSampler();

        void createImages(size_t imageCount);
        void createImageViews();
        void createDepthResources(size_t imageCount);

        std::vector<VkImage> images;
        std::vector<VmaAllocation> imageAllocations;
        std::vector<VkImageView> imageViews;

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;

        VkFormat colorFormat = VK_FORMAT_UNDEFINED;
        VkFormat depthFormat = VK_FORMAT_UNDEFINED;
        
        VkSampler imageSampler;

        uint32_t imageIndex{0};
    };
}
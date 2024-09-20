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
        GWOffscreenRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount);
        ~GWOffscreenRenderer();

        VkImage getCurrentImage() const {images[imageIndex];}
        VkImageView getCurrentImageView() const { return imageViews[imageIndex]; }

        void startOffscreenRenderPass(VkCommandBuffer commandBuffer);
        void endOffscreenRenderPass(VkCommandBuffer commandBuffer);

        void createNextImage();

        VkSampler getImageSampler() { return imageSampler; }
    private:
        GWindow &window;
        GWinDevice &device;

        void init(float imageCount);
        void createImageSampler();

        void createImages(float imageCount);
        void createImageViews();
        void createDepthResources(float imageCount);


        std::vector<VkImage> images;
        std::vector<VmaAllocation> imageAllocations;
        std::vector<VkImageView> imageViews;

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;

        VkFormat& depthFormat;
        
        VkSampler imageSampler;

        uint32_t imageIndex{0};
    };
}
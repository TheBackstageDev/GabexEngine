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
    class GWOffscreenRenderer
    {
    public:
        GWOffscreenRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount);
        ~GWOffscreenRenderer();

        VkRenderPass getRenderPass() const { return renderPass; }
        VkImage getCurrentImage() const { return images[imageIndex]; }
        VkImageView getCurrentImageView() const { return imageViews[imageIndex]; }

        void startOffscreenRenderPass(VkCommandBuffer commandBuffer);
        void endOffscreenRenderPass(VkCommandBuffer commandBuffer);

        void createNextImage();

        VkSampler getImageSampler() { return imageSampler; }
    private:
        GWindow &window;
        GWinDevice &device;

        void init(VkFormat depthFormat, float imageCount);
        void createImageSampler();

        void createImages(float imageCount);
        void createImageViews();
        void createDepthResources(VkFormat depthFormat);

        void createRenderPass(VkFormat depthFormat);
        void createFramebuffers();

        std::vector<VkImage> images;
        std::vector<VmaAllocation> imageAllocations;
        std::vector<VkImageView> imageViews;

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;

        std::vector<VkFramebuffer> frameBuffers;
        
        VkSampler imageSampler;
        VkRenderPass renderPass;

        float imageIndex{0};
    };
}
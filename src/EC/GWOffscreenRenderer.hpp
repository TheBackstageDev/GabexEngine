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
        GWOffscreenRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount, bool isShadowMap);
        ~GWOffscreenRenderer();

        VkRenderPass getRenderPass() const { return renderPass; }
        VkImage getCurrentImage() const { if (!isShadowMap) { return images[imageIndex]; } else { return depthImages[imageIndex]; }; }
        VkImageView getCurrentImageView() const { if (!isShadowMap) { return imageViews[imageIndex]; } else { return depthImageViews[imageIndex]; }; }

        void startOffscreenRenderPass(VkCommandBuffer commandBuffer);
        void endOffscreenRenderPass(VkCommandBuffer commandBuffer);

        void createNextImage();

        VkSampler getImageSampler() { return imageSampler; }
    private:
        GWindow &window;
        GWinDevice &device;

        void init(float imageCount, bool isShadowMap);
        void createImageSampler();

        void createImages(float imageCount);
        void createImageViews();
        void createDepthResources(float imageCount);

        void createRenderPass(bool isShadowMap);
        void createFramebuffers();

        std::vector<VkImage> images;
        std::vector<VmaAllocation> imageAllocations;
        std::vector<VkImageView> imageViews;

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;

        std::vector<VkFramebuffer> frameBuffers;

        VkFormat& depthFormat;
        
        VkSampler imageSampler;
        VkRenderPass renderPass;

        uint32_t imageIndex{0};

        bool isShadowMap{false};
    };
}
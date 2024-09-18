#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "GWRendererToolkit.hpp"

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

        VkRenderPass getRenderPass() const { return renderPass; }
        VkImage getCurrentImage() const { depthImages[imageIndex]; }
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

        void createRenderPass();
        void createFramebuffers();

        std::vector<VkImage> depthImages;
        std::vector<VmaAllocation> depthImagesAllocation;
        std::vector<VkImageView> depthImageViews;

        std::vector<VkFramebuffer> frameBuffers;

        VkFormat &depthFormat;

        VkSampler imageSampler;
        VkRenderPass renderPass;

        uint32_t imageIndex{0};
    };
}
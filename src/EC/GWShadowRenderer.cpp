#include "GWShadowRenderer.hpp"

#include <stdexcept>
#include <iostream>

#define SHADOW_HEIGHT 1024
#define SHADOW_WIDTH 1024

namespace GWIN
{
    GWShadowRenderer::GWShadowRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount) : window(window), device(device), depthFormat(depthFormat)
    {
        init(imageCount);
    }

    GWShadowRenderer::~GWShadowRenderer()
    {
        vkDestroySampler(device.device(), imageSampler, nullptr);

        for (size_t i = 0; i < depthImages.size(); i++)
        {
            vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
            vkDestroyImage(device.device(), depthImages[i], nullptr);
            vmaFreeMemory(device.getAllocator(), depthImagesAllocation[i]);
        }
    }

    void GWShadowRenderer::init(float imageCount)
    {
        createImageSampler();
        createDepthResources(imageCount);
    }

    void GWShadowRenderer::createNextImage()
    {
        if (imageIndex >= depthImages.size() - 1)
        {
            imageIndex = 0;
            return;
        }

        imageIndex++;
    }

    void GWShadowRenderer::createImageSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = device.properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f; // Optional
        samplerInfo.minLod = 0.0f;     // Optional
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void GWShadowRenderer::createDepthResources(float imageCount)
    {
        depthImages.resize(imageCount);
        depthImagesAllocation.resize(imageCount);
        depthImageViews.resize(imageCount);

        for (size_t i = 0; i < depthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = SHADOW_WIDTH;
            imageInfo.extent.height = SHADOW_HEIGHT;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = depthFormat;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            device.createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, depthImages[i], depthImagesAllocation[i]);

            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = depthImages[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = depthFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &depthImageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create depth image views!");
            }
        }
    }

    void GWShadowRenderer::startOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        VkRenderingAttachmentInfoKHR depthAttachment{};
        depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        depthAttachment.imageView = depthImageViews[imageIndex];
        depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        VkClearValue depthClear = {{1.0f, 0}};
        depthAttachment.clearValue = depthClear;

        VkRenderingInfoKHR renderingInfo{};
        renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR;
        renderingInfo.renderArea.offset = {0, 0};
        renderingInfo.renderArea.extent = window.getExtent();
        renderingInfo.layerCount = 1;
        renderingInfo.colorAttachmentCount = 0;
        renderingInfo.pColorAttachments = nullptr;
        renderingInfo.pDepthAttachment = &depthAttachment;

        vkCmdBeginRenderingKHR(commandBuffer, &renderingInfo);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderingInfo.renderArea.extent.width);
        viewport.height = static_cast<float>(renderingInfo.renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{{0, 0}, window.getExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void GWShadowRenderer::endOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderingKHR(commandBuffer);
    }
}
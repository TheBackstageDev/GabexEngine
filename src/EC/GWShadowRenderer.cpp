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

        for (auto frameBuffer : frameBuffers)
        {
            vkDestroyFramebuffer(device.device(), frameBuffer, nullptr);
        }

        vkDestroyRenderPass(device.device(), renderPass, nullptr);
    }

    void GWShadowRenderer::init(float imageCount)
    {
        createImageSampler();
        createRenderPass();
        createDepthResources(imageCount);
        createFramebuffers();
    }

    void GWShadowRenderer::createFramebuffers()
    {
        frameBuffers.resize(depthImageViews.size());

        for (size_t i = 0; i < frameBuffers.size(); i++)
        {
            std::array<VkImageView, 1> attachments = {
                depthImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = SHADOW_WIDTH;
            framebufferInfo.height = SHADOW_HEIGHT;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen framebuffer!");
            }
        }
    }

    void GWShadowRenderer::createNextImage()
    {
        if (imageIndex >= depthImages.size() - 1)
        {
            for (size_t i = 0; i < depthImages.size(); ++i)
            {
                if (frameBuffers[i] != VK_NULL_HANDLE)
                {
                    vkDestroyFramebuffer(device.device(), frameBuffers[i], nullptr);
                    frameBuffers[i] = VK_NULL_HANDLE;
                }
            }

            for (size_t i = 0; i < frameBuffers.size(); i++)
            {
                std::array<VkImageView, 1> attachments = {
                    depthImageViews[i]};

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = SHADOW_WIDTH;
                framebufferInfo.height = SHADOW_HEIGHT;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create offscreen framebuffer!");
                }
            }

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

    void GWShadowRenderer::createRenderPass()
    {
        std::vector<VkAttachmentDescription> attachments(1);

        // Depth attachment
        attachments[0].format = depthFormat;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        GWIN::createRenderPass(device, renderPass, attachments, false, true);
    }

    void GWShadowRenderer::startOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = VkExtent2D{SHADOW_WIDTH, SHADOW_HEIGHT};

        VkClearValue depthClear = {{1.0f, 0}};

        std::array<VkClearValue, 1> clearValues = {depthClear};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(SHADOW_WIDTH);
        viewport.height = static_cast<float>(SHADOW_HEIGHT);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, window.getExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void GWShadowRenderer::endOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }
}
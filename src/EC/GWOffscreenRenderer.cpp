#include "GWOffscreenRenderer.hpp"

#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWOffscreenRenderer::GWOffscreenRenderer(GWindow &window, GWinDevice &device, VkFormat depthFormat, float imageCount) : window(window), device(device), depthFormat(depthFormat)
    {
        init(imageCount);
    }

    GWOffscreenRenderer::~GWOffscreenRenderer()
    {
        vkDestroySampler(device.device(), imageSampler, nullptr);

        for (auto imageView : imageViews)
        {
            vkDestroyImageView(device.device(), imageView, nullptr);
        }
        imageViews.clear();

        for (size_t i = 0; i < depthImages.size(); i++)
        {
            vkDestroyImageView(device.device(), depthImageViews[i], nullptr);
            vkDestroyImage(device.device(), depthImages[i], nullptr);
            vmaFreeMemory(device.getAllocator(), imageAllocations[i]);
            vmaFreeMemory(device.getAllocator(), depthImagesAllocation[i]);
        }
    }

    void GWOffscreenRenderer::init(float imageCount)
    {
        createImageSampler();
        createImages(imageCount);
        createImageViews();
        createDepthResources(imageCount);
    }

    void GWOffscreenRenderer::createNextImage()
    {
        if (imageIndex >= images.size() - 1)
        {
            for (size_t i = 0; i < images.size(); ++i)
            {

                if (imageViews[i] != VK_NULL_HANDLE)
                {
                    vkDestroyImageView(device.device(), imageViews[i], nullptr);
                    imageViews[i] = VK_NULL_HANDLE;
                }

                if (images[i] != VK_NULL_HANDLE)
                {
                    vmaDestroyImage(device.getAllocator(), images[i], imageAllocations[i]);
                    images[i] = VK_NULL_HANDLE;
                    imageAllocations[i] = VK_NULL_HANDLE;
                }
            }

            for (size_t i = 0; i < images.size(); ++i)
            {
                VkImageCreateInfo imageInfo{};
                imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
                imageInfo.imageType = VK_IMAGE_TYPE_2D;
                imageInfo.extent.width = window.getExtent().width;
                imageInfo.extent.height = window.getExtent().height;
                imageInfo.extent.depth = 1;
                imageInfo.mipLevels = 1;
                imageInfo.arrayLayers = 1;
                imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
                imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
                imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
                imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
                imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                VmaAllocationCreateInfo allocInfo{};
                allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

                if (vmaCreateImage(device.getAllocator(), &imageInfo, &allocInfo, &images[i], &imageAllocations[i], nullptr) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create offscreen image!");
                }
            }

            for (size_t i = 0; i < imageViews.size(); i++)
            {
                VkImageViewCreateInfo viewInfo{};
                viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                viewInfo.image = images[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
                viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                viewInfo.subresourceRange.baseMipLevel = 1;
                viewInfo.subresourceRange.levelCount = 1;
                viewInfo.subresourceRange.baseArrayLayer = 0;
                viewInfo.subresourceRange.layerCount = 1;

                if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
                {
                    throw std::runtime_error("Failed to create offscreen image views!");
                }
            }

            imageIndex = 0;
            return;
        }

        imageIndex++;
    }


    void GWOffscreenRenderer::createImageSampler()
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
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
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

    void GWOffscreenRenderer::createImages(float imageCount)
    {
        images.resize(imageCount);
        imageAllocations.resize(imageCount);

        for (size_t i = 0; i < images.size(); ++i)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = window.getExtent().width;
            imageInfo.extent.height = window.getExtent().height;
            imageInfo.extent.depth = 1;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(device.getAllocator(), &imageInfo, &allocInfo, &images[i], &imageAllocations[i], nullptr) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen image!");
            }
        }
    }

    void GWOffscreenRenderer::createImageViews()
    {
        imageViews.resize(images.size());

        for (size_t i = 0; i < imageViews.size(); i++)
        {
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = images[i];
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 1;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen image views!");
            }
        }
    }

    void GWOffscreenRenderer::createDepthResources(float imageCount)
    {
        depthImages.resize(imageCount);
        depthImagesAllocation.resize(imageCount);
        depthImageViews.resize(imageCount);

        for (size_t i = 0; i < depthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = 2000;
            imageInfo.extent.height = 2000;
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

    void GWOffscreenRenderer::startOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        VkRenderingAttachmentInfoKHR colorAttachment{};
        colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
        colorAttachment.imageView = imageViews[imageIndex];
        colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        VkClearValue clearColor = {{{0.01f, 0.0f, 0.0f, 1.0f}}};
        colorAttachment.clearValue = clearColor;

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
        renderingInfo.colorAttachmentCount = 1;
        renderingInfo.pColorAttachments = &colorAttachment;
        renderingInfo.pDepthAttachment = &depthAttachment;

        // Begin rendering with dynamic rendering
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

    void GWOffscreenRenderer::endOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderingKHR(commandBuffer);
    }
}
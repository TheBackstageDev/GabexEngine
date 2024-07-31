#include "GWOffscreenRenderer.hpp"

#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWOffscreenRenderer::GWOffscreenRenderer(GWindow& window, GWinDevice& device, VkFormat depthFormat, float imageCount) : window(window), device(device), depthFormat(depthFormat)
    {
        init(imageCount);
    }

    GWOffscreenRenderer::~GWOffscreenRenderer()
    {
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

        for (auto frameBuffer : frameBuffers)
        {
            vkDestroyFramebuffer(device.device(), frameBuffer, nullptr);
        }

        vkDestroyRenderPass(device.device(), renderPass, nullptr);
    }

    void GWOffscreenRenderer::init(float imageCount)
    {
        createImageSampler();
        createImages(imageCount);
        createImageViews();
        createRenderPass();
        createDepthResources();
        createFramebuffers();
    }

    void GWOffscreenRenderer::createFramebuffers()
    {
        frameBuffers.resize(imageViews.size());

        for (size_t i = 0; i < frameBuffers.size(); i++)
        {
            std::array<VkImageView, 2> attachments = {
                imageViews[i],
                depthImageViews[i]};

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = window.getExtent().width;
            framebufferInfo.height = window.getExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen framebuffer!");
            }
        }
    }

    void GWOffscreenRenderer::createNextImage()
    {
        if (frameBuffers[imageIndex] != VK_NULL_HANDLE)
        {
            vkDestroyFramebuffer(device.device(), frameBuffers[imageIndex], nullptr);
            frameBuffers[imageIndex] = VK_NULL_HANDLE;
        }

        if (imageViews[imageIndex] != VK_NULL_HANDLE)
        {
            vkDestroyImageView(device.device(), imageViews[imageIndex], nullptr);
            imageViews[imageIndex] = VK_NULL_HANDLE;
        }

        if (images[imageIndex] != VK_NULL_HANDLE)
        {
            vmaDestroyImage(device.getAllocator(), images[imageIndex], imageAllocations[imageIndex]);
            images[imageIndex] = VK_NULL_HANDLE;
            imageAllocations[imageIndex] = VK_NULL_HANDLE;
        }

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

        device.createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, images[imageIndex], imageAllocations[imageIndex]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = images[imageIndex];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageViews[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen image view!");
        }

        std::array<VkImageView, 2> attachments = {
            imageViews[imageIndex],
            depthImageViews[imageIndex]};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = window.getExtent().width;
        framebufferInfo.height = window.getExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &frameBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create offscreen framebuffer!");
        }

        if (imageIndex >= images.size() - 1)
        {
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
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create offscreen image views!");
            }
        }
    }

    void GWOffscreenRenderer::createDepthResources()
    {
        depthImages.resize(imageViews.size());
        depthImagesAllocation.resize(imageViews.size());
        depthImageViews.resize(imageViews.size());

        for (size_t i = 0; i < depthImages.size(); i++)
        {
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_2D;
            imageInfo.extent.width = window.getExtent().width;
            imageInfo.extent.height = window.getExtent().height;
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

    void GWOffscreenRenderer::createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = VK_FORMAT_R8G8B8A8_SRGB;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;

        if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create Offscreen render pass!");
        }
    }

    void GWOffscreenRenderer::startOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = window.getExtent();

        VkClearValue clearColor = {{{0.01f, 0.0f, 0.0f, 1.0f}}};
        VkClearValue depthClear = {{1.0f, 0}};

        std::array<VkClearValue, 2> clearValues = {clearColor, depthClear};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(renderPassInfo.renderArea.extent.width);
        viewport.height = static_cast<float>(renderPassInfo.renderArea.extent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, window.getExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }

    void GWOffscreenRenderer::endOffscreenRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }
}
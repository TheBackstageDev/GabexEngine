#include "GWCubemapHandler.hpp"

#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWCubemapHandler::GWCubemapHandler(GWinDevice& device) : device(device) {};

    GWCubemapHandler::~GWCubemapHandler()
    {
        for (auto &cubeMap : cubeMapsForDeletion)
        {
            vkDestroyImageView(device.device(), cubeMap.Cubeimage.imageView, nullptr);
            vmaDestroyImage(device.getAllocator(), cubeMap.Cubeimage.image, cubeMap.Cubeimage.allocation);
        }
    }

    CubeMap GWCubemapHandler::createCubeMap(CubeMapInfo& info)
    {
        ++lastCubemapId;

        CubeMap newCubeMap{};
        
        newCubeMap.info = info;

        newCubeMap.id = lastCubemapId;

        generateCubeMap(newCubeMap);

        cubeMapsForDeletion.push_back(std::move(newCubeMap));
        return newCubeMap;
    }

    void GWCubemapHandler::generateCubeMap(CubeMap& cubeMap)
    {
        int texWidth, texHeight, texChannels;
        VkDeviceSize imageSize;
        std::vector<stbi_uc *> pixels(6);

        auto& faces = cubeMap.info.getFaces();

        for (uint32_t i = 0; i < faces.size(); ++i)
        {
            pixels[i] = stbi_load(faces[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
            if (!pixels[i])
            {
                std::cout << faces[i] << std::endl;
                throw std::runtime_error("failed to load cubemap texture image!");
            }
        }

        imageSize = texWidth * texHeight * 4;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.extent.width = static_cast<uint32_t>(texWidth);
        imageInfo.extent.height = static_cast<uint32_t>(texHeight);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 6;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        device.createImageWithInfo(imageInfo, VMA_MEMORY_USAGE_GPU_ONLY, cubeMap.Cubeimage.image, cubeMap.Cubeimage.allocation);

        GWBuffer stagingBuffer{
            device,
            imageSize * 6,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_CPU_ONLY};

        stagingBuffer.map();
        for (size_t i = 0; i < pixels.size(); ++i)
        {
            stagingBuffer.writeToBuffer(static_cast<void *>(pixels[i]), imageSize, i * imageSize);
        }
        stagingBuffer.unmap();

        for (auto &pixel : pixels)
        {
            stbi_image_free(pixel);
        }

        transitionImageLayout(cubeMap, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device.copyBufferToImage(stagingBuffer.getBuffer(), cubeMap.Cubeimage.image, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), 6);
        transitionImageLayout(cubeMap, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        createImageView(cubeMap);
    }

    void GWCubemapHandler::createSampler(VkSampler& sampler)
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
        samplerInfo.minLod = 0.0f; // Optional
        samplerInfo.maxLod = 0.0f; // Optional

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void GWCubemapHandler::transitionImageLayout(CubeMap &cubeMap, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = cubeMap.Cubeimage.image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 6;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        device.endSingleTimeCommands(commandBuffer);

        cubeMap.Cubeimage.layout = newLayout;
    }

    void GWCubemapHandler::createImageView(CubeMap &cubeMap)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = cubeMap.Cubeimage.image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 6;

        if (vkCreateImageView(device.device(), &viewInfo, nullptr, &cubeMap.Cubeimage.imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create cubemap image view!");
        }
    }
}
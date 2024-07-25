#include "GWTexture.hpp"

#include <stdexcept>
#include <iostream>
#include <numeric>

namespace GWIN
{
    GWTexture::GWTexture(const std::string &pathToTexture, GWImageLoader &imageLoader, GWinDevice &device)
        : imageLoader(imageLoader), device(device)
    {
        textureImage = imageLoader.loadImage(pathToTexture);

        createSampler();
        createDescriptorSet();
    }

    GWTexture::~GWTexture() 
    {
        vkDestroySampler(device.device(), sampler, nullptr);
        vmaDestroyImage(device.getAllocator(), textureImage.image, textureImage.allocation);
    }

    void GWTexture::createDescriptorSet()
    {

    }

    void GWTexture::createSampler()
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
        samplerInfo.maxLod = static_cast<uint32_t>(textureImage.mipLevels);

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }
}
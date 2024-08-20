#include "GWTextureHandler.hpp"

#include <stdexcept>
#include <iostream>
#include <numeric>

namespace GWIN
{
    GWTextureHandler::GWTextureHandler(GWImageLoader &imageLoader, GWinDevice &device)
        : imageLoader(imageLoader), device(device)
    {
    }

    GWTextureHandler::~GWTextureHandler()
    {
        for (uint32_t i = 0; i < textures.size(); ++i)
        {
            auto currentTexture = textures[i];
            vkDestroySampler(device.device(), currentTexture.textureSampler, nullptr);
            vkDestroyImageView(device.device(), currentTexture.textureImage.imageView, nullptr);
            vmaDestroyImage(device.getAllocator(), currentTexture.textureImage.image, currentTexture.textureImage.allocation);
        }
    }

    Texture GWTextureHandler::createTexture(std::string& pathToTexture, bool mipMap)
    {
        Texture texture{};

        ++lastTextureId;
        texture.id = lastTextureId;
        
        texture.textureImage = imageLoader.loadImage(pathToTexture, mipMap);
        
        createSampler(texture.textureImage.mipLevels, texture.textureSampler);

        textures.push_back(std::move(texture));

        return texture;
    }

    void GWTextureHandler::destroyTexture(uint32_t id)
    {
        auto currentTexture = textures[id - 1];
        vkDestroySampler(device.device(), currentTexture.textureSampler, nullptr);
        vkDestroyImageView(device.device(), currentTexture.textureImage.imageView, nullptr);
        vmaDestroyImage(device.getAllocator(), currentTexture.textureImage.image, currentTexture.textureImage.allocation);
    }

    void GWTextureHandler::createSampler(uint32_t mipLevels, VkSampler& sampler)
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
        samplerInfo.maxLod = static_cast<uint32_t>(mipLevels);

        if (vkCreateSampler(device.device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void GWTextureHandler::changeImageLayout(Texture &texture, VkImageLayout newLayout)
    {
        imageLoader.transitionImageLayout(texture.textureImage, newLayout);
    }

    std::vector<TextureInfo> GWTextureHandler::getTextures() const
    {
        std::vector<TextureInfo> info;
        
        for (auto& texture : textures)
        {
            info.push_back({texture.pathToTexture, texture.id});
        }

        return info;
    }
}
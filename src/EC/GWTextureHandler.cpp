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
        }
    }

    Texture GWTextureHandler::createTexture(std::string &pathToTexture, bool mipMap, TextureType type)
    {
        ++lastTextureId;
        
        Texture texture{};

        texture.id = lastTextureId;
        
        texture.textureImage = imageLoader.loadImage(pathToTexture, mipMap, type == TEXTURE_TYPE_DIFFUSE ? VK_FORMAT_R8G8B8A8_SRGB : VK_FORMAT_R8G8B8A8_UNORM);
        texture.pathToTexture = pathToTexture;

        GWIN::createSampler(device, texture.textureSampler, texture.textureImage.mipLevels);

        textures.push_back(std::move(texture));

        return texture;
    }

    void GWTextureHandler::destroyTexture(uint32_t id)
    {
        auto currentTexture = textures[id];
        vkDestroySampler(device.device(), currentTexture.textureSampler, nullptr);
        imageLoader.destroyImage(currentTexture.textureImage.id);
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
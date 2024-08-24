#pragma once

#include "GWImageLoader.hpp"
#include "GWDescriptors.hpp"
#include "../GWBuffer.hpp"

#include <string>
#include <memory>

namespace GWIN
{
    enum TextureType
    {
        TEXTURE_TYPE_DIFFUSE,
        TEXTURE_TYPE_NORMAL,
        TEXTURE_TYPE_SPECULAR,
        TEXTURE_TYPE_ROUGHNESS,
        TEXTURE_TYPE_DISPLACEMENT,
        TEXTURE_TYPE_AMBIENT,
    };

    struct TextureInfo
    {
        std::string pathToTexture;
        uint32_t id;
    };

    struct Texture
    {
        VkSampler textureSampler = VK_NULL_HANDLE;
        Image textureImage;
        std::string pathToTexture;
        uint32_t id;
    };

    class GWTextureHandler
    {
    public:
        GWTextureHandler(GWImageLoader &imageLoader, GWinDevice &device);
        ~GWTextureHandler();

        GWTextureHandler(const GWTextureHandler &) = delete;
        GWTextureHandler &operator=(const GWTextureHandler &) = delete;

        Texture createTexture(std::string &pathToTexture, bool mipMap);
        void destroyTexture(uint32_t id);
        void changeImageLayout(Texture& texture, VkImageLayout newLayout);

        GWImageLoader getImageLoader() { return imageLoader; }
        void createSampler(uint32_t mipLevels, VkSampler &sampler);

        std::vector<TextureInfo> getTextures() const;
        void resetTextures() { textures.clear(); lastTextureId = 0; };

    private:
        GWinDevice& device;
        GWImageLoader& imageLoader;

        std::vector<Texture> textures;

        uint32_t lastTextureId{0};

        void createDescriptorSet(Texture &texture);
    };
} // namespace GWIN

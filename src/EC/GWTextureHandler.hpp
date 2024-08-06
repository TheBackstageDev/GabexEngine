#pragma once

#include "GWImageLoader.hpp"
#include "GWDescriptors.hpp"
#include "../GWBuffer.hpp"

#include <memory>

namespace GWIN
{
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

        Texture createTexture(std::string &pathToTexture);
        void changeImageLayout(Texture& texture, VkImageLayout newLayout);

        GWImageLoader getImageLoader() { return imageLoader; }
        void createSampler(uint32_t mipLevels, VkSampler &sampler);

    private:
        GWinDevice& device;
        GWImageLoader& imageLoader;

        std::vector<Texture> textures;

        uint32_t lastTextureId{0};

        void createDescriptorSet(Texture &texture);
    };
} // namespace GWIN

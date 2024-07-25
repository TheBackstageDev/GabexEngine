#pragma once

#include "GWImageLoader.hpp"
#include "GWDescriptors.hpp"
#include "../GWBuffer.hpp"

#include <memory>

namespace GWIN
{
    class GWTexture
    {
    public:
        GWTexture(const std::string &pathToTexture, GWImageLoader &imageLoader, GWinDevice &device);
        ~GWTexture();

        GWTexture(const GWTexture &) = delete;
        GWTexture &operator=(const GWTexture &) = delete;


        Image getImage() { return textureImage; }
        VkImageView getImageView() { return textureImage.imageView; }
        VkSampler getSampler() { return sampler; }
        VkImageLayout getimageLayout() { return textureImage.layout; }

        VkDescriptorSet getTextureSet() { return textureSet; }

    private:
        GWinDevice& device;
        GWImageLoader& imageLoader;

        void createSampler();
        void createDescriptorSet();

        std::unique_ptr<GWBuffer> TextureBuffer;

        Image textureImage{};
        VkDescriptorSet textureSet;
        VkSampler sampler;
    };
} // namespace GWIN

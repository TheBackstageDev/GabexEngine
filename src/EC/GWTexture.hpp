#pragma once

#include "GWImageLoader.hpp"
#include "GWDescriptors.hpp"

namespace GWIN
{
    class GWTexture
    {
    public:
        GWTexture(const std::string &pathToTexture, GWImageLoader &imageLoader, GWinDevice &device);
        ~GWTexture();

        Image getImage() { return textureImage; }
        VkImageView getImageView() { return textureImage.imageView; }
        VkSampler getSampler() { return sampler; }
        VkImageLayout getimageLayout() { return textureImage.layout; }

    private:
        GWinDevice& device;
        GWImageLoader& imageLoader;

        void createSampler();

        Image textureImage{};
        VkSampler sampler;
    };
} // namespace GWIN

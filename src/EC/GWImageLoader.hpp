#pragma once

#include "stb/std_image.hpp"
#include "stb/stb_image_write.h"

#include <string>

#include "../GWDevice.hpp"
#include "../GWBuffer.hpp"
#include "vma/vk_mem_alloc.h"

namespace GWIN
{
    struct Image
    {
        VkFormat format{VK_FORMAT_R8G8B8A8_SRGB};
        VkExtent2D size;
        uint32_t mipLevels;
        VmaAllocation allocation;
        VkImage image;
        VkImageView imageView;
        VkImageLayout layout{VK_IMAGE_LAYOUT_UNDEFINED};
    };

    class GWImageLoader
    {
    public:
        GWImageLoader(GWinDevice &device);
        ~GWImageLoader();

        Image loadImage(const std::string &filepath, bool isMipMapped);

        void transitionImageLayout(Image &image, VkImageLayout newLayout);

    private:
        GWinDevice& device;

        void createImage(
            VkExtent2D imageProps,
            VkDeviceSize imageSize,
            VkFormat format,
            VkImageUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VkImage &image,
            VmaAllocation &allocation,
            uint32_t mipLevels);

        void createImageView(Image& image);
        void generateMipMaps(Image& image);

        std::vector<Image> imagesForDeletion;
    };
}
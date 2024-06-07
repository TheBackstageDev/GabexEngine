#pragma once

#include <stdexcept>
#include <vector>
#include <optional>
#include <set>

#include <vulkan/vulkan.h>
#include "GWPipeLine.hpp"

namespace GWIN
{
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete()
        {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class GWinDevice
    {
    public:
        GWinDevice(VkInstance vk_instance, VkSurfaceKHR surface, short height, short width);
        ~GWinDevice();

        GWinDevice(const GWinDevice &) = delete;
        void operator=(const GWinDevice &) = delete;

        void Init();
        void updateWindowValues(short height, short width);

    private:
        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);

        const bool enableValidationLayers = true;
        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        void createLogicalDevice();
        void createSwapChain();
        void createImageViews();

        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
        VkPhysicalDevice PHYDevice = VK_NULL_HANDLE;
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        short height;
        short width;

        std::vector<VkImage> swapChainImages;
        VkSwapchainKHR swapChain;
        VkFormat swapChainImageFormat;
        std::vector<VkImageView> swapChainImageViews;
        VkExtent2D swapChainExtent;
        VkDevice LOGDevice;
        VkQueue graphicsQueue;
        VkQueue presentQueue;
        VkInstance instance;
        VkSurfaceKHR surface;
        GPipeLine *pipeline;
    };
}
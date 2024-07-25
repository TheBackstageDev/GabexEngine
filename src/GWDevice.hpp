#pragma once

#include "GWindow.hpp"
#include "vma/vk_mem_alloc.h"

#include <string>
#include <vector>

namespace GWIN
{

    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct QueueFamilyIndices
    {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    class GWinDevice
    {
    public:
        GWinDevice(GWindow &window);
        ~GWinDevice();

        // Not copyable or movable
        GWinDevice(const GWinDevice &) = delete;
        void operator=(const GWinDevice &) = delete;
        GWinDevice(GWinDevice &&) = delete;
        GWinDevice &operator=(GWinDevice &&) = delete;

        VkCommandPool getCommandPool() { return commandPool; }
        VkDevice device() { return device_; }
        VkInstance getInstance() { return instance; }
        VkPhysicalDevice phyDevice() { return physicalDevice; }
        VkSurfaceKHR surface() { return surface_; }
        VkQueue graphicsQueue() { return graphicsQueue_; }
        VkQueue presentQueue() { return presentQueue_; }
        VkSampleCountFlagBits getMaxSamples() { return msaaSamples; }

        SwapChainSupportDetails getSwapChainSupport() { return querySwapChainSupport(physicalDevice); }
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        QueueFamilyIndices findPhysicalQueueFamilies() { return findQueueFamilies(physicalDevice); }
        VkFormat findSupportedFormat(
            const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

        // Buffer Helper Functions
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VmaMemoryUsage memoryUsage,
            VkBuffer &buffer,
            VmaAllocation &bufferAllocation);

        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
        void copyBufferToImage(
            VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
        VkDeviceMemory getBufferMemory(VmaAllocation buffer);

        void createImageWithInfo(
            const VkImageCreateInfo &imageInfo,
            VmaMemoryUsage memoryUsage,
            VkImage &image,
            VmaAllocation &imageAllocation);

        VmaAllocator getAllocator()
        {
            return allocator_;
        };

        VkPhysicalDeviceProperties properties;

    private:
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void createCommandPool();

        //VMA
        void createAllocator();

        // helper functions
        bool isDeviceSuitable(VkPhysicalDevice device);
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void hasGflwRequiredInstanceExtensions();
        bool checkDeviceExtensionSupport(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
        VkSampleCountFlagBits getMaxUsableSampleCount();

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        GWindow &window;
        VkCommandPool commandPool;
        VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocator allocator_;
        VkDevice device_;
        VkSurfaceKHR surface_;
        VkQueue graphicsQueue_;
        VkQueue presentQueue_;

        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};
        const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME};
    };

} // namespace GWIN
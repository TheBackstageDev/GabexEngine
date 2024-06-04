#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>
#include <optional>

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

namespace GWIN
{
    class GWinDevice
    {
    public:
        GWinDevice(VkInstance vk_instance);
        ~GWinDevice();

        void Init();

    private:
        void pickPhysicalDevice();
        bool isDeviceSuitable(VkPhysicalDevice device);

        const bool enableValidationLayers = true;
        const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

        void createLogicalDevice();

        VkPhysicalDevice PHYDevice = VK_NULL_HANDLE;
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

        VkDevice LOGDevice;

        VkInstance instance;
    };
}
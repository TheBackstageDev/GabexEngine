#pragma once

#include <vulkan/vulkan.h>
#include <stdexcept>
#include <vector>

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

        VkPhysicalDevice PHYDevice = VK_NULL_HANDLE;

        VkInstance instance;
    };
}
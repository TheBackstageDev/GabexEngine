#include "GWDevice.hpp"

namespace GWIN
{
    GWinDevice::GWinDevice(VkInstance vk_instance) : instance(vk_instance)
    {
        Init();
    }

    GWinDevice::~GWinDevice()
    {
    }

    void GWinDevice::Init()
    {
        pickPhysicalDevice();
    }

    void GWinDevice::pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        for (const auto &device : devices)
        {
            if (isDeviceSuitable(device))
            {
                PHYDevice = device;
                break;
            }
        }

        if (PHYDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool GWinDevice::isDeviceSuitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
               deviceFeatures.geometryShader;
    }
}
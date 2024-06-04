#include <vector>
#include <string>
#include <vulkan/vulkan.h>

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifndef NDEBUG
const bool enableValidationLayers = true;
#else
const bool enableValidationLayers = false;
#endif

namespace GWIN
{
    class GWDebug
    {
    public:
        GWDebug(VkInstance vk_instance);
        ~GWDebug();

        void setupDebugMessenger();
        static bool checkValidationLayerSupport();
        static void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);
        static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator);

    private:
        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
            void *pUserData);
    };
}
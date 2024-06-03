#pragma once

#define GLFW_INCLUDE_VULKAN
#include <string>
#include <cstring>
#include <vector>
#include <glfw/include/GLFW/glfw3.h>

const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

namespace GWIN
{
    class GWindow
    {
    public:
        GWindow(short height, short width, std::string name);
        ~GWindow();

        GWindow(const GWindow &) = delete;
        GWindow &operator=(const GWindow &) = delete;

        void run();

    private:
        void initWindow();
        void initVulkan();

        void createVKInstance();

        // Validation Layers
        std::vector<const char *> getRequiredExtensions();
        bool checkValidationLayerSupport();
        void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
        void setupDebugMessenger();
        VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger);

        short height;
        short width;
        std::string windowName;

        GLFWwindow *Window;

        VkInstance instance;
        VkDebugUtilsMessengerEXT debugMessenger;
    };
}

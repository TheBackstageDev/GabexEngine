#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/include/GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3native.h>

#include <string>
#include <vector>
#include "GWDevice.hpp"
#include "GWDebug.hpp"

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

        void createSurface();

        std::vector<const char *> getRequiredExtensions();

        short height;
        short width;
        std::string windowName;

        GLFWwindow *window;
        VkInstance instance;
        GWDebug *debug;
        VkSurfaceKHR surface;
        GWinDevice *device;
    };
}

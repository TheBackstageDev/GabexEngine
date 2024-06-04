#pragma once

#define GLFW_INCLUDE_VULKAN
#include <string>
#include <vector>
#include <glfw/include/GLFW/glfw3.h>
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
        std::vector<const char *> getRequiredExtensions();

        short height;
        short width;
        std::string windowName;

        GLFWwindow *Window;
        VkInstance instance;
        GWDebug *debug;
        GWinDevice *device;
    };
}

#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <glfw/include/GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3native.h>

#include <string>
#include <vector>

namespace GWIN
{
    class GWindow
    {
    public:
        GWindow(short height, short width, std::string name);
        ~GWindow();

        GWindow(const GWindow &) = delete;
        GWindow &operator=(const GWindow &) = delete;

        void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        bool shouldClose()
        {
            printf("%c", glfwWindowShouldClose(window));
            return glfwWindowShouldClose(window);
        }
        VkExtent2D getExtent() { return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}; }

    private:
        void initWindow();
        std::vector<const char *> getRequiredExtensions();

        short height;
        short width;
        std::string windowName;

        GLFWwindow *window;
    };
}

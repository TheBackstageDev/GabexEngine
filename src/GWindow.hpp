#pragma once

#define GLFW_INCLUDE_VULKAN
#include <string>
#include <glfw/include/GLFW/glfw3.h>

namespace GWIN
{
    class GWindow
    {
    public:
        GWindow(short height, short width, std::string name);
        ~GWindow();

        bool ShouldClose() { return glfwWindowShouldClose(Window); }

    private:
        void initWindow();
        short height;
        short width;
        std::string windowName;

        GLFWwindow *Window;
    };
}

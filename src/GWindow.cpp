#include "GWindow.hpp"
#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWindow::GWindow(short height, short width, std::string name) : width{width}, height{height}, windowName{name}
    {
        initWindow();
    }

    GWindow::~GWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void GWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
    }

    void GWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to craete window surface");
        }
    }
}

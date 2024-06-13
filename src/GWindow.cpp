#include "GWindow.hpp"
#include <stdexcept>
#include <iostream>

namespace GWIN
{
    GWindow::GWindow(short height, short width, std::string name) : width{width}, height{height}, windowName{name}
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, frameBufferResizeCallback);
    }

    GWindow::~GWindow()
    {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void GWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to craete window surface");
        }
    }

    void GWindow::frameBufferResizeCallback(GLFWwindow *window, int width, int height)
    {
        auto GWindow = reinterpret_cast<GWIN::GWindow *>(glfwGetWindowUserPointer(window));
        GWindow->frameBufferResized = true;
        GWindow->width = width;
        GWindow->height = height;
    }
}

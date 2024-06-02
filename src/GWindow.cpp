#include "GWindow.hpp"

namespace GWIN
{
    GWindow::GWindow(short height, short width, std::string name) : height(height), width(width), windowName(name)
    {
        initWindow();
    }

    GWindow::~GWindow()
    {
        glfwDestroyWindow(Window);
        glfwTerminate();
    }

    void GWindow::initWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        Window = glfwCreateWindow((int)width, (int)height, windowName.c_str(), nullptr, nullptr);
    }
}
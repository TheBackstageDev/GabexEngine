#include "FirstApp.hpp"

namespace GWIN
{
    void FirstApp::run()
    {
        while (!gWindow.ShouldClose())
        {
            glfwPollEvents();
        }
    };
}
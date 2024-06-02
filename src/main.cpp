// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// GWin

#include "FirstApp.hpp"

int main()
{
    GWIN::FirstApp app;

    try
    {
        app.run();
    }
    catch (const std::exception *e)
    {
        std::cerr << e->what() << "\n";
    }

    return 0;
}
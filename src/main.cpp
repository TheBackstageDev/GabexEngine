// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// GWin

#include "GWindow.hpp"

int main()
{
    const uint32_t HEIGHT = 500;
    const uint32_t WIDTH = 800;
    GWIN::GWindow app(WIDTH, HEIGHT, "Gabex Engine");

    try
    {
        app.run();
    }
    catch (const std::exception *e)
    {
        std::cerr << e->what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
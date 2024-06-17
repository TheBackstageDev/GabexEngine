// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

// GWin

#include "GWApp.hpp"

int main()
{
    GWIN::GWapp app{};
    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        std::cin;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
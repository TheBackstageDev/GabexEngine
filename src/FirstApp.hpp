#pragma once

#include "GWindow.hpp"

namespace GWIN
{
    class FirstApp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 500;

        void run();

        FirstApp() : gWindow(HEIGHT, WIDTH, "Gabex Engine") {}

    private:
        GWindow gWindow;
    };
}

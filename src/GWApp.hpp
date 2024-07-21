#pragma once

#include "./systems/MasterRenderSystem.hpp"

namespace GWIN
{
    class GWapp
    {
    public:
        static constexpr int WIDTH = 1500;
        static constexpr int HEIGHT = 1000;

        GWapp() {};

        GWapp(const GWapp &) = delete;
        GWapp &operator=(const GWapp &) = delete;

        void run();

    private:
        GWindow GWindow{WIDTH, HEIGHT, "Gabex Engine"};
        GWinDevice GDevice{GWindow};
        MasterRenderSystem renderSystem{GWindow, GDevice};
    };
}
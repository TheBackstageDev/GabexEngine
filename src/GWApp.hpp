#pragma once

#include "./ECSSystems/GWRenderer.hpp"
#include "./ECSSystems/RenderSystem.hpp"

// std
#include <memory>
#include <vector>

namespace GWIN
{
    class GWapp
    {
    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 800;

        GWapp();

        GWapp(const GWapp &) = delete;
        GWapp &operator=(const GWapp &) = delete;

        void run();

    private:
        void loadGameObjects();

        GWindow GWindow{WIDTH, HEIGHT, "Gabex Engine"};
        GWinDevice GDevice{GWindow};
        GWRenderer GRenderer{GWindow, GDevice};

        std::vector<GWGameObject> gameObjects;
    };
}
#pragma once

#include "GWindow.hpp"
#include "GWDevice.hpp"
#include "GWRenderer.hpp"
#include "GWPipeLine.hpp"
#include "GWGameObject.hpp"

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
        ~GWapp();

        GWapp(const GWapp &) = delete;
        GWapp &operator=(const GWapp &) = delete;

        void run();

    private:
        void createPipelineLayout();
        void createPipeline();
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);

        VkPipelineLayout pipelineLayout;

        GWindow GWindow{WIDTH, HEIGHT, "Gabex Engine"};
        GWinDevice GDevice{GWindow};
        GWRenderer GRenderer{GWindow, GDevice};

        std::unique_ptr<GPipeLine> Pipeline;
        std::vector<GWGameObject> gameObjects;
    };
}
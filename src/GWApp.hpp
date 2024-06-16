#pragma once

#include "GWindow.hpp"
#include "GWDevice.hpp"
#include "GWSwapChain.hpp"
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
        void createCommandBuffers();
        void drawFrame();
        void loadGameObjects();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;

        GWindow GWindow{WIDTH, HEIGHT, "Gabex Engine"};
        GWinDevice GDevice{GWindow};
        std::unique_ptr<GWinSwapChain> swapChain;
        std::unique_ptr<GPipeLine> Pipeline;
        std::vector<GWGameObject> gameObjects;
    };
}
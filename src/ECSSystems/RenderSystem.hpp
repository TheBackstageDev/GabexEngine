#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWPipeLine.hpp"
#include "GWGameObject.hpp"
#include "GWCamera.hpp"

// std
#include <memory>
#include <vector>
#include <stdexcept>

namespace GWIN
{
    class RenderSystem
    {
    public:
        RenderSystem(GWinDevice &device, VkRenderPass renderPass, bool isWireFrame);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GWGameObject> &gameObjects, const GWCamera& camera);
    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass, bool isWireFrame);

        VkPipelineLayout pipelineLayout;

        GWinDevice& GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
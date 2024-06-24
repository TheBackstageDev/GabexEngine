#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWPipeLine.hpp"
#include "../EC/GWFrameInfo.hpp"
#include "../EC/GWGameObject.hpp"
#include "../EC/GWCamera.hpp"

// std
#include <memory>
#include <vector>
#include <stdexcept>

namespace GWIN
{
    class RenderSystem
    {
    public:
        RenderSystem(GWinDevice &device, VkRenderPass renderPass, bool isWireFrame, VkDescriptorSetLayout globalSetLayout );
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(FrameInfo& frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass, bool isWireFrame);

        VkPipelineLayout pipelineLayout;

        GWinDevice& GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
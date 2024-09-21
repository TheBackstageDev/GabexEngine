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

#include "../GWSwapChain.hpp"

namespace GWIN
{
    class RenderSystem
    {
    public:
        RenderSystem(GWinDevice &device, bool isWireFrame, std::vector<VkDescriptorSetLayout> setLayouts);
        ~RenderSystem();

        RenderSystem(const RenderSystem &) = delete;
        RenderSystem &operator=(const RenderSystem &) = delete;

        void renderGameObjects(FrameInfo& frameInfo);

        VkPipeline getPipeline() const { return Pipeline->pipeline(); };
    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
        void createPipeline(bool isWireFrame);

        VkPipelineLayout pipelineLayout;

        GWinDevice& GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
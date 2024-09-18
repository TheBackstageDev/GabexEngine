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
    class ShadowSystem
    {
    public:
        ShadowSystem(GWinDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
        ~ShadowSystem();

        ShadowSystem(const ShadowSystem &) = delete;
        ShadowSystem &operator=(const ShadowSystem &) = delete;

        void render(FrameInfo &frameInfo);

        VkPipeline getPipeline() const { return Pipeline->pipeline(); };

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
        void createPipeline(VkRenderPass renderPass);

        VkPipelineLayout pipelineLayout;

        GWinDevice &GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWPipeLine.hpp"
#include "GWFrameInfo.hpp"
#include "GWGameObject.hpp"
#include "GWCamera.hpp"

// std
#include <memory>
#include <vector>
#include <stdexcept>

namespace GWIN
{
    class PointLightSystem
    {
    public:
        PointLightSystem(GWinDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout );
        ~PointLightSystem();

        PointLightSystem(const PointLightSystem &) = delete;
        PointLightSystem &operator=(const PointLightSystem &) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& Ubo);
        void render(FrameInfo& frameInfo);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VkPipelineLayout pipelineLayout;

        GWinDevice& GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
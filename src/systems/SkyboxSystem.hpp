#pragma once

#include "GWFrameInfo.hpp"
#include "GWCubemapHandler.hpp"
#include "../GWPipeLine.hpp"

namespace GWIN
{
    class SkyboxSystem
    {
    public:
        SkyboxSystem(GWinDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts);
        ~SkyboxSystem();

        SkyboxSystem(const SkyboxSystem &) = delete;
        SkyboxSystem &operator=(const SkyboxSystem &) = delete;

        void render(FrameInfo &frameInfo);

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
        void createPipeline(VkRenderPass renderPass);

        VkPipelineLayout pipelineLayout;
        std::unique_ptr<GPipeLine> pipeline;

        GWinDevice& device;
    };
} // namespace GWIN

#pragma once

#include "GWFrameInfo.hpp"
#include "GWCubemapHandler.hpp"
#include "../GWPipeLine.hpp"

namespace GWIN
{
    class SkyboxSystem
    {
    public:
        SkyboxSystem(GWinDevice &device, std::vector<VkDescriptorSetLayout> setLayouts);
        ~SkyboxSystem();

        SkyboxSystem(const SkyboxSystem &) = delete;
        SkyboxSystem &operator=(const SkyboxSystem &) = delete;

        void render(FrameInfo &frameInfo);

        void setSkybox(VkDescriptorSet& skybox, uint32_t id) { currentSkybox = skybox; id = id; }
        uint32_t getSkyboxID() { return id; }

    private:
        void createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts);
        void createPipeline();

        VkPipelineLayout pipelineLayout;
        std::unique_ptr<GPipeLine> pipeline;

        VkDescriptorSet currentSkybox = VK_NULL_HANDLE;
        uint32_t id;

        GWinDevice& device;
    };
} // namespace GWIN

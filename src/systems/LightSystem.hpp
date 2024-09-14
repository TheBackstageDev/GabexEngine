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
    class LightSystem
    {
    public:
        LightSystem(GWinDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout );
        ~LightSystem();

        LightSystem(const LightSystem &) = delete;
        LightSystem &operator=(const LightSystem &) = delete;

        void update(FrameInfo& frameInfo, GlobalUbo& Ubo);
        void render(FrameInfo& frameInfo);

        void calculateLightMatrix(std::array<glm::mat4, 6> &matrix, float aspect, float Near, float Far);
        void calculateDirectionalLightMatrix(glm::mat4& matrix, float Near, float Far);
    private:
        void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
        void createPipeline(VkRenderPass renderPass);

        VkPipelineLayout pipelineLayout;

        GWinDevice& GDevice;
        std::unique_ptr<GPipeLine> Pipeline;
    };
}
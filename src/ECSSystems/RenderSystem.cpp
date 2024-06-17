#include "RenderSystem.hpp"

#include <glm/gtc/constants.hpp>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    RenderSystem::RenderSystem(GWinDevice &device, VkRenderPass renderPass) : GDevice(device)
    {
        createPipelineLayout();
        createPipeline(renderPass);
    }

    RenderSystem::~RenderSystem()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout()
    {
        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(SpushConstant);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

        if (vkCreatePipelineLayout(GDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Create Pipeline Layout!");
        }
    }

    void RenderSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        Pipeline = std::make_unique<GPipeLine>(
            GDevice,
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.vert.spv",
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.frag.spv",
            pipelineConfig);
    }

    void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GWGameObject> &gameObjects)
    {
        // render
        Pipeline->bind(commandBuffer);
        for (auto &obj : gameObjects)
        {
            SpushConstant push{};
            push.offset = obj.transform2d.translation;
            push.color = obj.color;
            push.transform = obj.transform2d.mat2();

            vkCmdPushConstants(
                commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SpushConstant),
                &push);
            obj.model->bind(commandBuffer);
            obj.model->draw(commandBuffer);
        }
    }
}
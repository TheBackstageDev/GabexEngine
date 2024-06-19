#include "RenderSystem.hpp"

#include <glm/gtc/constants.hpp>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat4 transform{1.f};
        alignas(16) glm::vec3 color;
    };

    RenderSystem::RenderSystem(GWinDevice &device, VkRenderPass renderPass, bool isWireFrame) : GDevice(device)
    {
        createPipelineLayout();
        createPipeline(renderPass, isWireFrame);
    }

    RenderSystem::~RenderSystem()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout()
    {
        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
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

    void RenderSystem::createPipeline(VkRenderPass renderPass, bool isWireFrame)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        if (isWireFrame)
        {
            pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
        }

        Pipeline = std::make_unique<GPipeLine>(
            GDevice,
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.vert.spv",
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.frag.spv",
            pipelineConfig);
    }

    void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GWGameObject> &gameObjects, const GWCamera& camera)
    {
        // render
        Pipeline->bind(commandBuffer);

        auto projectionView = camera.getProjection() * camera.getView();

        for (auto &obj : gameObjects)
        {
            obj.transform.rotation.y += glm::mod(0.01f, glm::two_pi<float>());
            SpushConstant push{};
            push.color = obj.color;
            push.transform = projectionView * obj.transform.mat4();

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
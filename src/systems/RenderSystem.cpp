#include "RenderSystem.hpp"

#include <glm/gtc/constants.hpp>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat4 modelMatrix{1.f};
    };

    RenderSystem::RenderSystem(GWinDevice &device, VkRenderPass renderPass, bool isWireFrame, VkDescriptorSetLayout globalSetLayout ) : GDevice(device)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass, isWireFrame);
    }

    RenderSystem::~RenderSystem()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void RenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstant.size = sizeof(SpushConstant);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
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
            "C:/Users/viega/Desktop/CoisaDoGabriel/GabexEngine/src/shaders/shader.vert.spv",
            "C:/Users/viega/Desktop/CoisaDoGabriel/GabexEngine/src/shaders/shader.frag.spv",
            pipelineConfig);
    }

    void RenderSystem::renderGameObjects(FrameInfo& frameInfo)
    {
        // render
        Pipeline->bind(frameInfo.commandBuffer);

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr
        );

        for (auto &kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if (obj.model == nullptr) continue;

            SpushConstant push{};
            push.modelMatrix = obj.transform.mat4();

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SpushConstant),
                &push);
            obj.model->bind(frameInfo.commandBuffer);
            obj.model->draw(frameInfo.commandBuffer);
        }
    }
}
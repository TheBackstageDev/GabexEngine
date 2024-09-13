#include "SkyboxSystem.hpp"

#include <stdexcept>
#include <cassert>
#include "iostream"

#include <filesystem>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat4 modelMatrix{1.f};
    };

    SkyboxSystem::SkyboxSystem(GWinDevice &device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout> setLayouts)
    : device(device)
    {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass);
    }

    SkyboxSystem::~SkyboxSystem()
    {
        vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
        vkDestroyPipeline(device.device(), pipeline->pipeline(), nullptr);
    }

    void SkyboxSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts)
    {
        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstant.offset = 0;
        pushConstant.size = sizeof(SpushConstant);
        
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
        pipelineLayoutInfo.pSetLayouts = setLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

        if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Create Pipeline Layout!");
        }
    }

    void SkyboxSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;
        pipelineConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        pipelineConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
        pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        pipelineConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        pipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineConfig.rasterizationInfo.depthClampEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        pipelineConfig.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineConfig.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipelineConfig.rasterizationInfo.depthBiasEnable = VK_FALSE;

        pipeline = std::make_unique<GPipeLine>(
            device,
            "../src/shaders/skybox.vert.spv",
            "../src/shaders/skybox.frag.spv",
            pipelineConfig);

        if (!pipeline)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    void SkyboxSystem::render(FrameInfo& frameInfo)
    {
        assert(pipeline && "Pipeline must be created before calling render");

        if (currentSkybox == VK_NULL_HANDLE)
            return;

        auto &skybox = frameInfo.currentInfo.gameObjects.at(0);

        pipeline->bind(frameInfo.commandBuffer);

        VkDescriptorSet descriptorSets[] = {frameInfo.globalDescriptorSet, currentSkybox};

        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 
            2, 
            descriptorSets,
            0,
            nullptr);

        SpushConstant push{};
        push.modelMatrix = skybox.transform.mat4();

        vkCmdPushConstants(
            frameInfo.commandBuffer,
            pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SpushConstant),
            &push);

        auto& model = frameInfo.currentInfo.meshes.at(skybox.model);
        model->bind(frameInfo.commandBuffer);
        model->draw(frameInfo.commandBuffer);
    }
}
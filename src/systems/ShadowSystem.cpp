#include "ShadowSystem.hpp"

#include <iostream>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat4 modelMatrix{1.f};
        //glm::mat4 lightViewProj{0.f};
    };

    ShadowSystem::ShadowSystem(GWinDevice &device, std::vector<VkDescriptorSetLayout> setLayouts)
        : GDevice(device)
    {
        createPipelineLayout(setLayouts);
        createPipeline();
    }

    ShadowSystem::~ShadowSystem()
    {
        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
        }
    }

    void ShadowSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts)
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

        if (vkCreatePipelineLayout(GDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to Create Pipeline Layout!");
        }
    }

    void ShadowSystem::createPipeline()
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        // GPipeLine::enableAlphaBlending(pipelineConfig);
        pipelineConfig.pipelineLayout = pipelineLayout;

        Pipeline = std::make_unique<GPipeLine>(
            GDevice,
            "src/shaders/shadow.vert.spv",
            "src/shaders/shadow.frag.spv",
            pipelineConfig);

        if (!Pipeline)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    void ShadowSystem::render(FrameInfo &frameInfo)
    {
        assert(Pipeline && "Pipeline must be created before calling renderGameObjects");

        Pipeline->bind(frameInfo.commandBuffer);
        vkCmdBindDescriptorSets(
            frameInfo.commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout,
            0, 1,
            &frameInfo.globalDescriptorSet,
            0,
            nullptr);

        for (auto &kv : frameInfo.currentInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.model == -1 || obj.getName() == "Skybox")
                continue;

            auto &model = frameInfo.currentInfo.meshes.at(obj.model);

            SpushConstant push{};
            push.modelMatrix = obj.transform.mat4();
            //push.lightViewProj

            vkCmdPushConstants(
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0,
                sizeof(SpushConstant),
                &push);

            if (model->hasSubModels())
            {
                for (auto &subModel : model->getSubModels())
                {
                    subModel->bind(frameInfo.commandBuffer);
                    subModel->draw(frameInfo.commandBuffer);
                }
            }

            model->bind(frameInfo.commandBuffer);
            model->draw(frameInfo.commandBuffer);
        }
    }
}
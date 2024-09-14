#include "RenderSystem.hpp"

#include <glm/gtc/constants.hpp>
#include <iostream>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat4 modelMatrix{1.f};
        uint32_t MaterialIndex;
    };

    RenderSystem::RenderSystem(GWinDevice &device, VkRenderPass renderPass, bool isWireFrame, std::vector<VkDescriptorSetLayout> setLayouts)
        : GDevice(device)
    {
        createPipelineLayout(setLayouts);
        createPipeline(renderPass, isWireFrame);
    }

    RenderSystem::~RenderSystem()
    {
        if (pipelineLayout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
        }
    }

    void RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout> setLayouts)
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
            "../src/shaders/shader.vert.spv",
            "../src/shaders/shader.frag.spv",
            pipelineConfig);

        if (!Pipeline)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    void RenderSystem::renderGameObjects(FrameInfo &frameInfo)
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

        std::unordered_map<uint32_t, VkDescriptorSet> textureBindings;

        for (auto &kv : frameInfo.currentInfo.gameObjects)
        {
            auto &obj = kv.second;
            if (obj.model == -1 || obj.getName() == "Skybox")
                continue;

            if (frameInfo.flags.frustumCulling && !frameInfo.currentInfo.currentCamera.isPointInFrustum(obj.transform.translation))
                continue;

            auto &model = frameInfo.currentInfo.meshes.at(obj.model);
            uint32_t textureToBind = model->Textures[0];

            if (textureBindings.find(textureToBind) == textureBindings.end())
            {
                VkDescriptorSet textureDescriptorSet = frameInfo.currentInfo.textures[textureToBind];
                textureBindings[textureToBind] = textureDescriptorSet;

                vkCmdBindDescriptorSets(
                    frameInfo.commandBuffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout,
                    1, 1,
                    &textureDescriptorSet,
                    0,
                    nullptr);
            }

            SpushConstant push{};
            push.modelMatrix = obj.transform.mat4();
            push.MaterialIndex = model->Material;

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
                    uint32_t subTextureToBind = subModel->Textures[0]; 

                    if (textureBindings.find(subTextureToBind) == textureBindings.end())
                    {
                        VkDescriptorSet subTextureDescriptorSet = frameInfo.currentInfo.textures[subTextureToBind];
                        textureBindings[subTextureToBind] = subTextureDescriptorSet;

                        vkCmdBindDescriptorSets(
                            frameInfo.commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipelineLayout,
                            1, 1,
                            &subTextureDescriptorSet,
                            0,
                            nullptr);
                    }

                    subModel->bind(frameInfo.commandBuffer);
                    subModel->draw(frameInfo.commandBuffer);
                }
            }

            model->bind(frameInfo.commandBuffer);
            model->draw(frameInfo.commandBuffer);
        }
    }
}
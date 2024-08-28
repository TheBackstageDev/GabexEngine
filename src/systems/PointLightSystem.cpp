#include "PointLightSystem.hpp"

#include <glm/gtc/constants.hpp>
#include <iostream>
namespace GWIN
{
    struct pointLightPushConstant
    {
        glm::vec4 position{1.f};
        glm::vec4 color{1.f};
        float radius;
    };

    PointLightSystem::PointLightSystem(GWinDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout ) : GDevice(device)
    {
        createPipelineLayout(globalSetLayout);
        createPipeline(renderPass);
    }

    PointLightSystem::~PointLightSystem()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
    {
        VkPushConstantRange pushConstant{};
        pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstant.size = sizeof(pointLightPushConstant);

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

    void PointLightSystem::createPipeline(VkRenderPass renderPass)
    {
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        PipelineConfigInfo pipelineConfig{};
        GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
        pipelineConfig.renderPass = renderPass;
        pipelineConfig.pipelineLayout = pipelineLayout;

        pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;

        Pipeline = std::make_unique<GPipeLine>(
            GDevice,
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/point_light.vert.spv",
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/point_light.frag.spv",
            pipelineConfig);
    }

    void PointLightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
        int lightIndex = 0;
        for (auto& kv : frameInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.light == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            if (obj.light->cutOffAngle == 0.f)
            {
                ubo.lights[lightIndex].Position = glm::vec4(obj.transform.translation, 0.f);
                ubo.lights[lightIndex].Direction = glm::vec4(glm::eulerAngles(obj.transform.rotation), 0.0f);
            } else {
                ubo.lights[lightIndex].Position = glm::vec4(obj.transform.translation, 1.f);
                ubo.lights[lightIndex].Direction = glm::vec4(glm::eulerAngles(obj.transform.rotation), glm::cos(obj.light->cutOffAngle));
            }

            ubo.lights[lightIndex].Color = glm::vec4(obj.color, obj.light->lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }

    void PointLightSystem::render(FrameInfo& frameInfo)
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

        for(auto& kv : frameInfo.gameObjects)
        {
            auto& obj = kv.second;
            if(obj.light == nullptr) continue;

            pointLightPushConstant push{};
            push.position = glm::vec4(obj.transform.translation, 1.f);
            push.color = glm::vec4(obj.color, obj.light->lightIntensity);
            push.radius = obj.transform.scale;
            
            vkCmdPushConstants
            (
                frameInfo.commandBuffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, 
                sizeof(pointLightPushConstant),
                &push
            );
            
            vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
        }
    }
}
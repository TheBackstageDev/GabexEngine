#include "SkyboxSystem.hpp"

#include <stdexcept>
#include <cassert>

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

        pipeline = std::make_unique<GPipeLine>(
            device,
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/skybox.vert.spv",
            "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/skybox.frag.spv",
            pipelineConfig);

        if (!pipeline)
        {
            throw std::runtime_error("Failed to create graphics pipeline!");
        }
    }

    void SkyboxSystem::render(FrameInfo& frameInfo)
    {
        assert(pipeline && "Pipeline must be created before calling render");

        pipeline->bind(frameInfo.commandBuffer);
    }
}
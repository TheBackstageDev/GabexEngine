#pragma once

#include <vector>
#include <vulkan/vulkan.h>
#include "GWDevice.hpp"

namespace GWIN
{
    struct PipelineConfigInfo
    {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class GPipeLine
    {
    public:
        GPipeLine(
            GWinDevice &device,
            const std::string &vertFilepath,
            const std::string &fragFilepath,
            const PipelineConfigInfo &configInfo);
        ~GPipeLine();

        GPipeLine(const GPipeLine &) = delete;
        void operator=(const GPipeLine &) = delete;

        void bind(VkCommandBuffer commandBuffer);

        static void defaultPipelineConfigInfo(
            PipelineConfigInfo &configInfo, uint32_t width, uint32_t height);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(
            const std::string &vertFilepath,
            const std::string &fragFilepath,
            const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        GWinDevice &gDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}
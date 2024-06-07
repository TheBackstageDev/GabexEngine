#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace GWIN
{
    class GPipeLine
    {
    public:
        GPipeLine(VkDevice device, VkExtent2D swapChainExtent);
        ~GPipeLine();

    private:
        void init();
        void createGraphicsPipeline();
        VkShaderModule createShaderModule(const std::vector<char> &code);

        VkExtent2D swapChainExtent;
        VkDevice LOGDevice;
        VkPipelineLayout pipelineLayout;
    };
}
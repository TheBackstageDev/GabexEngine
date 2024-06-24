#pragma once

#include <vulkan/vulkan.h>
#include "GWCamera.hpp"
#include "GWGameObject.hpp"

namespace GWIN
{
    #define MAX_LIGHTS 10

    struct Pointlight
    {
        glm::vec4 Position; //ignore W
        glm::vec4 Color; //W is intensity
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.2f}; //w is intensity
        Pointlight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float deltaTime;
        VkCommandBuffer commandBuffer;
        GWCamera& camera;
        VkDescriptorSet globalDescriptorSet;
        GWGameObject::map &gameObjects;
    };
}
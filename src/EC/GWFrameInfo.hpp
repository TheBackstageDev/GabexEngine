#pragma once

#include <vulkan/vulkan.h>
#include "GWCamera.hpp"
#include "GWGameObject.hpp"

namespace GWIN
{
    #define MAX_LIGHTS 20
    #define MAX_MATERIALS 100

    struct Light
    {
        glm::vec4 Position; //w is type; 0 - Point, 1 - Spot
        glm::vec4 Color; //W is intensity
        glm::vec4 Direction;
    };

    struct Material
    {
        glm::vec4 color; // w is intensity
        alignas(16) glm::vec3 data; // x is metallic, y is roughness, z is id
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 sunLight{0.f, glm::radians(45.f), 0.f, .5f};  // w is intensity
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.5f}; //w is intensity
        Light lights[MAX_LIGHTS];
        Material materials[MAX_MATERIALS];
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
        VkDescriptorSet currentFrameSet;
    };

    struct GameObjectInfo
    {
        std::string objName{"DefaultName"};
        std::string filePath;
        float scale{1.f};
        glm::vec3 position{0.f, .5f, 0.f}; 
        VkDescriptorSet texture = VK_NULL_HANDLE;
    };
}
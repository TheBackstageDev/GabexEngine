#pragma once

#include <volk/volk.h>
#include "GWCamera.hpp"
#include "GWGameObject.hpp"
#include <vector>
#include <array>

namespace GWIN
{
    #define MAX_LIGHTS 20
    #define MAX_MATERIALS 100

    struct Light
    {
        glm::vec4 Position; //w is type; 0 - Point, 1 - Spot
        glm::vec4 Color; //W is intensity
        glm::vec4 Direction; // W is cutoff angle
    };

    struct Material
    {
        glm::vec4 color; // w is intensity
        alignas(16) glm::vec3 data; // x is metallic, y is roughness, z is id
    };

    struct LightBuffer
    {
        Light lights[MAX_LIGHTS];
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, 0.3f}; //w is intensity
        int numLights;
    };

    struct MaterialBuffer
    {
        Material materials[MAX_MATERIALS];
    };

    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::mat4 inverseView{1.f};
        glm::vec4 sunLight{0.f, 0.f, 0.f, .5f};  // w is intensity
        glm::mat4 sunLightSpaceMatrix{0.f};
        DeviceAddress light;
        DeviceAddress material;
        float exposure;
        bool renderShadows;
    };

    struct SceneInfo
    {
        GWCamera &currentCamera;
        GWGameObject::map &gameObjects;
        GWModel::map &meshes;
        VkDescriptorSet& textures;
    };

    struct FrameFlags
    {
        bool frustumCulling{false};
    };

    struct FrameInfo
    {
        int frameIndex;
        float deltaTime;
        VkCommandBuffer commandBuffer;
        VkDescriptorSet globalDescriptorSet;
        SceneInfo& currentInfo;
        VkDescriptorSet currentFrameSet;
        VkDescriptorSet shadowMapSet;
        FrameFlags flags;
    };
}
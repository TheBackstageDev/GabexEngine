#include "LightSystem.hpp"

#include <glm/gtc/constants.hpp>
#include <iostream>

const uint32_t SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

namespace GWIN
{
    glm::mat4 LightSystem::calculateDirectionalLightMatrix(glm::vec3 cameraPosition, glm::vec3 lightRotation)
    {
        glm::vec3 lightPosition = cameraPosition - glm::normalize(lightRotation) * 1.2f;

        glm::mat4 lightProjection = glm::orthoLH(-20.f, 20.f, -20.f, 20.f, 0.1f, 100.0f);

        glm::mat4 lightView = glm::lookAtLH(lightPosition, cameraPosition, glm::vec3(0.0f, -1.0f, 0.0f));

        return lightProjection * lightView;
    }

    void LightSystem::calculateLightMatrix(std::array<glm::mat4, 6> &matrix, float aspect, float Near, float Far)
    {
        glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspect, Near, Far);
    }

    void LightSystem::update(FrameInfo& frameInfo, GlobalUbo& ubo) {
        int lightIndex = 0;

        auto& camera = frameInfo.currentInfo.currentCamera;

        for (auto& kv : frameInfo.currentInfo.gameObjects) {
            auto& obj = kv.second;
            if (obj.light == nullptr) continue;

            assert(lightIndex < MAX_LIGHTS && "Point lights exceed maximum specified");

            auto& light = ubo.lights[lightIndex];

            if (obj.light->cutOffAngle == 0.f)
            {
                light.Position = glm::vec4(obj.transform.translation, 0.f);
                light.Direction = glm::vec4(obj.transform.getRotation(), 0.0f);
            } else {
                light.Position = glm::vec4(obj.transform.translation, 1.f);
                light.Direction = glm::vec4(obj.transform.getRotation(), glm::cos(obj.light->cutOffAngle));
            }

            calculateLightMatrix(light.lightSpaceMatrix, SHADOW_WIDTH / SHADOW_HEIGHT, camera.getNearClip(), camera.getFarClip());
            light.Color = glm::vec4(obj.color, obj.light->lightIntensity);

            lightIndex += 1;
        }
        ubo.numLights = lightIndex;
    }
}
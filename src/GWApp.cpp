#include "GWApp.hpp"
#include "GWBuffer.hpp"
#include "./EC/GWFrameInfo.hpp"
#include "./EC/GWCamera.hpp"
#include "./EC/keyboard_movement_controller.hpp"

#include "./systems/RenderSystem.hpp"
#include "./systems/PointLightSystem.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include <numeric>
#include <iostream>
#include <chrono>

namespace GWIN
{
    GWapp::GWapp()
    {
        globalPool = GWDescriptorPool::Builder(GDevice)
        .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
        loadGameObjects();
    }

    void GWapp::run()
    {
        auto minOffsetAlligment = std::lcm(
            GDevice.properties.limits.minUniformBufferOffsetAlignment,
            GDevice.properties.limits.nonCoherentAtomSize
        );
        
        GWBuffer globalUboBuffer{
            GDevice, 
            sizeof(GlobalUbo), 
            GWinSwapChain::MAX_FRAMES_IN_FLIGHT, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            minOffsetAlligment
        };
        globalUboBuffer.map();

        auto globalSetLayout = GWDescriptorSetLayout::Builder(GDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .build();

        std::vector<VkDescriptorSet> globalDescriptorSets{GWinSwapChain::MAX_FRAMES_IN_FLIGHT};

        for(int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = globalUboBuffer.descriptorInfo();
            GWDescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
        }

        RenderSystem renderSystem{GDevice, GRenderer.getRenderPass(), false, globalSetLayout->getDescriptorSetLayout()};
        RenderSystem WireFramerenderSystem{GDevice, GRenderer.getRenderPass(), true, globalSetLayout->getDescriptorSetLayout()};

        PointLightSystem pointLightSystem{
            GDevice,
            GRenderer.getRenderPass(),
            globalSetLayout->getDescriptorSetLayout()
        };

        GWCamera camera{};

        bool isWireFrameActivated = false;

        auto viewerObject = GWGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5;
        keyboardMovementController cameraController{};
        auto currentTime = std::chrono::high_resolution_clock::now();

        int frameCount = 0;
        float totalTime = 0.0f;

        while (!GWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(GWindow.getWindow(), deltaTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = GRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

            totalTime += deltaTime;
            frameCount++;

            if (totalTime >= 1.0f)
            {
                float fps = static_cast<float>(frameCount) / totalTime;
                std::cout << "FPS: " << fps << std::endl;

                frameCount = 0;
                totalTime = 0.0f;
            }

            if (glfwGetKey(GWindow.getWindow(), cameraController.keys.activateWireframe))
            {
                isWireFrameActivated = true;
            }

            if (auto commandBuffer = GRenderer.startFrame())
            {
                int frameIndex = GRenderer.getFrameIndex();
                
                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects
                };

                //update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                globalUboBuffer.writeToIndex(&ubo, frameIndex);
                globalUboBuffer.flushIndex(frameIndex);
                
                //render
                GRenderer.startSwapChainRenderPass(commandBuffer);
                
                if (isWireFrameActivated)
                {
                    WireFramerenderSystem.renderGameObjects(frameInfo);
                }
                else
                {
                    renderSystem.renderGameObjects(frameInfo);
                }

                pointLightSystem.render(frameInfo);


                GRenderer.endSwapChainRenderPass(commandBuffer);
                GRenderer.endFrame();
            }

            isWireFrameActivated = false;
        }

        vkDeviceWaitIdle(GDevice.device());
    }

    void GWapp::loadGameObjects()
    {
        GWModelLoader modelLoader{GDevice};
        std::shared_ptr<GWModel> Model;
        modelLoader.importFile("C:/Users/viega/Desktop/CoisaDoGabriel/GabexEngine/src/models/vase.obj", Model);

        auto model = GWGameObject::createGameObject();
        model.model = Model;
        model.transform.translation = {0.f, .5f, 0.f};
        model.transform.rotation.z = .5f * glm::two_pi<float>();
        model.transform.scale = 1.f;

        auto model2 = GWGameObject::createGameObject();
        model2.model = Model;
        model2.transform.translation = {-1.f, .5f, 0.f};
        model2.transform.rotation.z = .5f * glm::two_pi<float>();
        model2.transform.scale = 1.f;

        modelLoader.importFile("C:/Users/viega/Desktop/CoisaDoGabriel/GabexEngine/src/models/quad.obj", Model);
        auto quad = GWGameObject::createGameObject();
        quad.model = Model;
        quad.transform.translation = {0.f, .5f, 0.f};
        quad.transform.scale = 3.f;

        gameObjects.emplace(model.getId(), std::move(model));
        gameObjects.emplace(model2.getId(), std::move(model2));
        gameObjects.emplace(quad.getId(), std::move(quad));

        
        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}  //
        };

        for (int i = 0; i < lightColors.size(); i++) {
            auto pointLight = GWGameObject::createPointLight(1.f, 0.05f, lightColors[i]);
            auto rotateLight = glm::rotate(
                glm::mat4(1.f),
                (i * glm::two_pi<float>()) / lightColors.size(),
                {0.f, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}
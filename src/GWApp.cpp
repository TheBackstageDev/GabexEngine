#include "GWApp.hpp"
#include "./ECSSystems/GWCamera.hpp"
#include "./ECSSystems/keyboard_movement_controller.hpp"
#include "GWBuffer.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <chrono>

namespace GWIN
{
    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
    };

    GWapp::GWapp()
    {
        loadGameObjects();
    }

    void GWapp::run()
    {
        GWBuffer globalUboBuffer{
            GDevice, 
            sizeof(GlobalUbo), 
            GWinSwapChain::MAX_FRAMES_IN_FLIGHT, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            GDevice.properties.limits.minUniformBufferOffsetAlignment
        };
        globalUboBuffer.map();

        RenderSystem renderSystem{GDevice, GRenderer.getRenderPass(), false};
        RenderSystem WireFramerenderSystem{GDevice, GRenderer.getRenderPass(), true};


        GWCamera camera{};

        bool isWireFrameActivated = false;

        auto viewerObject = GWGameObject::createGameObject();
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

                //update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                globalUboBuffer.writeToIndex(&ubo, frameIndex);
                globalUboBuffer.flushIndex(frameIndex);

                //render
                GRenderer.startSwapChainRenderPass(commandBuffer);

                if (isWireFrameActivated)
                {
                    WireFramerenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                }
                else
                {
                    renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
                }

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
        modelLoader.importFile("C:/Users/viega/Desktop/CoisaDoGabriel/GabexEngine/src/models/Suzane.obj", Model);

        auto Suzane = GWGameObject::createGameObject();
        Suzane.model = Model;
        Suzane.transform.translation = {0.f, 0.f, 2.5f};
        Suzane.transform.rotation.y = .5f * glm::two_pi<float>();
        Suzane.transform.scale = .5f;

        gameObjects.push_back(std::move(Suzane));
    }
}
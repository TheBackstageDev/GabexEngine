#include "GWApp.hpp"
#include "./ECSSystems/GWCamera.hpp"
#include "./ECSSystems/keyboard_movement_controller.hpp"

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <chrono>

namespace GWIN
{
    GWapp::GWapp()
    {
        loadGameObjects();
    }

    void GWapp::run()
    {
        RenderSystem renderSystem{GDevice, GRenderer.getRenderPass(), false};
        RenderSystem WireFramerenderSystem{GDevice, GRenderer.getRenderPass(), true};

        GWCamera camera{};

        auto viewerObject = GWGameObject::createGameObject();
        keyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        bool isWireFrameActivated = false;

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

            if (glfwGetKey(GWindow.getWindow(), cameraController.keys.activateWireframe))
            {
                isWireFrameActivated = true;
            }

            if (auto commandBuffer = GRenderer.startFrame())
            {
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

    // temporary helper function, creates a 1x1x1 cube centered at offset with an index buffer
    std::unique_ptr<GWModel> createCubeModel(GWinDevice &device, glm::vec3 offset)
    {
        GWModel::Builder modelBuilder{};
        modelBuilder.vertices = {
            // left face (white)
            {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
            {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},

            // right face (yellow)
            {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},

            // top face (orange, remember y axis points down)
            {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
            {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},

            // bottom face (red)
            {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
            {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},

            // nose face (blue)
            {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
            {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},

            // tail face (green)
            {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
            {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
        };
        for (auto &v : modelBuilder.vertices)
        {
            v.position += offset;
        }

        modelBuilder.indices = {
            0, 2, 1, 0, 1, 3,       // left face
            4, 6, 5, 4, 5, 7,       // right face
            8, 10, 9, 8, 9, 11,     // top face
            12, 14, 13, 12, 13, 15, // bottom face
            16, 18, 17, 16, 17, 19, // nose face
            20, 22, 21, 20, 21, 23  // tail face
        };

        return std::make_unique<GWModel>(device, modelBuilder);
    }

    void GWapp::loadGameObjects()
    {
        for (int i = 1; i <= 2; i++)
        {
            std::shared_ptr<GWModel> Model = createCubeModel(GDevice, {0.f, 0.f, 0.f});

            auto Cube = GWGameObject::createGameObject();
            Cube.model = Model;
            Cube.transform.translation = {0.f, 0.f, 2.5f * i};
            Cube.transform.scale = {.5f, .5f, .5f};

            gameObjects.push_back(std::move(Cube));
        }

    }
}
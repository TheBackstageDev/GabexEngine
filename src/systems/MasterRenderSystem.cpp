#include "MasterRenderSystem.hpp"

#include "../GWBuffer.hpp"
#include "../EC/GWModelLoader.hpp"

#include <numeric>
#include <iostream>
#include <chrono>
#include <iostream>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include <cassert>

namespace GWIN
{
    MasterRenderSystem::MasterRenderSystem(GWindow& window, GWinDevice& device)
        : window(window), device(device)
    {
        renderer = std::make_unique<GWRenderer>(window, device);
        offscreenRenderer = std::make_unique<GWOffscreenRenderer>(window, device, renderer->getSwapChainDepthFormat(), renderer->getImageCount());
        //offscreenRenderer = std::make_unique<GWOffscreenRenderer>(window, device);
        loadGameObjects();
        initialize();

        //Initializes GUI
        interfaceSystem = std::make_unique<GWInterface>(window, device, renderer->getSwapChainImageFormat());
        interfaceSystem->setLoadGameObjectCallback([this](GameObjectInfo& objectInfo) {
            loadGameObject(objectInfo);
        });
    }

    void MasterRenderSystem::initialize()
    {
        globalPool = GWDescriptorPool::Builder(device)
                         .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        auto globalSetLayout = GWDescriptorSetLayout::Builder(device)
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                   .build();

        auto minOffsetAlignment = std::lcm(
            device.properties.limits.minUniformBufferOffsetAlignment,
            device.properties.limits.nonCoherentAtomSize);

        globalUboBuffer = std::make_unique<GWBuffer>(
            device,
            sizeof(GlobalUbo),
            GWinSwapChain::MAX_FRAMES_IN_FLIGHT,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VMA_MEMORY_USAGE_CPU_TO_GPU,
            minOffsetAlignment);

        globalUboBuffer->map();
        
        globalDescriptorSets.resize(GWinSwapChain::MAX_FRAMES_IN_FLIGHT);

        /* const std::string pathToTexture = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/viking_room.png";
        std::unique_ptr<GWTexture> texture = std::make_unique<GWTexture>(pathToTexture, imageLoader, device);

        VkDescriptorImageInfo imageInfo{}; 
        imageInfo.imageView = texture->getImageView();
        imageInfo.sampler = texture->getSampler();
        imageInfo.imageLayout = texture->getimageLayout();  */

        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = globalUboBuffer->descriptorInfo();
            GWDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                //.writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        } 

        renderSystem = std::make_unique<RenderSystem>(device, renderer->getRenderPass(), false, globalSetLayout->getDescriptorSetLayout());
        wireframeRenderSystem = std::make_unique<RenderSystem>(device, renderer->getRenderPass(), true, globalSetLayout->getDescriptorSetLayout());
        pointLightSystem = std::make_unique<PointLightSystem>(device, renderer->getRenderPass(), globalSetLayout->getDescriptorSetLayout());
    }

    void MasterRenderSystem::updateCamera(GWGameObject& viewerObject, float deltaTime)
    {
        cameraController.moveInPlaneXZ(window.getWindow(), deltaTime, viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = renderer->getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
    }

    void MasterRenderSystem::run()
    {

        const std::string pathToTexture = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/viking_room.png";
        std::unique_ptr<GWTexture> texture = std::make_unique<GWTexture>(pathToTexture, imageLoader, device); 

        auto viewerObject = GWGameObject::createGameObject("Viewer Object");
        viewerObject.transform.translation.z = -2.5;
        gameObjects.emplace(viewerObject.getId(), std::move(viewerObject));
        
        currentTime = std::chrono::high_resolution_clock::now();

        while (!window.shouldClose())
        {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            updateCamera(viewerObject, deltaTime);

            if (auto commandBuffer = renderer->startFrame())
            {
                int frameIndex = renderer->getFrameIndex();

                // update
                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects,
                    false};

                GlobalUbo ubo{};
                ubo.projection = frameInfo.camera.getProjection();
                ubo.view = frameInfo.camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem->update(frameInfo, ubo);
                globalUboBuffer->writeToIndex(&ubo, frameIndex);
                globalUboBuffer->flushIndex(frameIndex);

                if (glfwGetKey(window.getWindow(), cameraController.keys.activateWireframe))
                {
                    frameInfo.isWireFrame = true;
                }

                offscreenRenderer->startOffscreenRenderPass(commandBuffer);

                if (frameInfo.isWireFrame)
                {
                    wireframeRenderSystem->renderGameObjects(frameInfo);
                }
                else
                {
                    renderSystem->renderGameObjects(frameInfo);
                }

                pointLightSystem->render(frameInfo);

               // offscreenRenderer->createNextImage();
                offscreenRenderer->endOffscreenRenderPass(commandBuffer);

                offscreenRenderer->createNextImage();
                offscreenImageDescriptor = ImGui_ImplVulkan_AddTexture(
                    offscreenRenderer->getImageSampler(),
                    offscreenRenderer->getCurrentImageView(),
                    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);

                // render
                interfaceSystem->newFrame(frameInfo);

               // std::cout << imageToDraw << " Generated Image \n";
                renderer->startSwapChainRenderPass(commandBuffer);
                if (ImGui::Begin("Viewport", nullptr))
                {
                    if (offscreenImageDescriptor)
                    {
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        ImGui::Image((ImTextureID)offscreenImageDescriptor, ImVec2(windowSize.x * 1.6, windowSize.y));
                    }
                    ImGui::End();  
                } 

                interfaceSystem->render(commandBuffer);
                renderer->endSwapChainRenderPass(commandBuffer);
                renderer->endFrame();

                ImGui_ImplVulkan_RemoveTexture(offscreenImageDescriptor);
                offscreenImageDescriptor = VK_NULL_HANDLE;

                frameInfo.isWireFrame = false;
            }
        }
    }

    void MasterRenderSystem::loadGameObject(GameObjectInfo& objectInfo)
    {
        std::shared_ptr<GWModel> model;
        modelLoader.importFile(objectInfo.filePath, model, false);

        auto obj = GWGameObject::createGameObject(objectInfo.objName);
        obj.model = model;
        obj.transform.translation = objectInfo.position;
        obj.transform.scale = objectInfo.scale;

        gameObjects.emplace(obj.getId(), std::move(obj));
    }

    void MasterRenderSystem::loadGameObjects()
    {
        std::shared_ptr<GWModel>
            Model;
        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/viking_room.obj", Model, false);

        auto model = GWGameObject::createGameObject("Viking Room");
        model.model = Model;
        model.transform.translation = {0.f, .5f, 1.f};
        model.transform.rotation.x = .25f * glm::two_pi<float>();
        model.transform.scale = 1.f;

        gameObjects.emplace(model.getId(), std::move(model));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f}
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
#include "MasterRenderSystem.hpp"

#include "../GWBuffer.hpp"

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

        cubemapHandler = std::make_unique<GWCubemapHandler>(device);

        initialize();
        loadGameObjects();

        //Initializes GUI
        interfaceSystem = std::make_unique<GWInterface>(window, device, renderer->getSwapChainImageFormat(), textureHandler);
        interfaceSystem->setLoadGameObjectCallback([this](GameObjectInfo& objectInfo) {
            loadGameObject(objectInfo);
        });

        interfaceSystem->setCreateTextureCallback([this](VkDescriptorSet& set, Texture& texture) {
            createSet(set, texture);
        });
    }

    void MasterRenderSystem::createSet(VkDescriptorSet &set, Texture &texture)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = texture.textureImage.layout;
        imageInfo.imageView = texture.textureImage.imageView;
        imageInfo.sampler = texture.textureSampler;

        auto bufferInfo = globalUboBuffer->descriptorInfo();

        GWDescriptorWriter(*textureSetLayout, *texturePool)
            .writeImage(0, &imageInfo)
            .build(set);
    }

    void MasterRenderSystem::initialize()
    {
        globalPool = GWDescriptorPool::Builder(device)
                         .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .build();

        auto globalSetLayout = GWDescriptorSetLayout::Builder(device)
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .build();

        texturePool = GWDescriptorPool::Builder(device)
                          .setMaxSets(1000)
                          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                          .build();

        textureSetLayout = GWDescriptorSetLayout::Builder(device)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                               .build();

        std::vector<VkDescriptorSetLayout> setLayouts = {globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};

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

        textureHandler = std::make_unique<GWTextureHandler>(imageLoader, device);

        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = globalUboBuffer->descriptorInfo();
            GWDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        renderSystem = std::make_unique<RenderSystem>(device, offscreenRenderer->getRenderPass(), false, setLayouts);
        wireframeRenderSystem = std::make_unique<RenderSystem>(device, offscreenRenderer->getRenderPass(), true, setLayouts);
        pointLightSystem = std::make_unique<PointLightSystem>(device, offscreenRenderer->getRenderPass(), globalSetLayout->getDescriptorSetLayout());
        skyboxSystem = std::make_unique<SkyboxSystem>(device, offscreenRenderer->getRenderPass(), setLayouts);
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
                    VK_NULL_HANDLE, false};

                GlobalUbo ubo{};
                ubo.projection = frameInfo.camera.getProjection();
                ubo.view = frameInfo.camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.sunLight = interfaceSystem->getLightDirection();
                pointLightSystem->update(frameInfo, ubo);
                globalUboBuffer->writeToIndex(&ubo, frameIndex);
                globalUboBuffer->flushIndex(frameIndex);

                if (glfwGetKey(window.getWindow(), cameraController.keys.activateWireframe))
                {
                    frameInfo.isWireFrame = true;
                }

                offscreenRenderer->startOffscreenRenderPass(commandBuffer);

                skyboxSystem->render(frameInfo);

                if (frameInfo.isWireFrame)
                {
                    wireframeRenderSystem->renderGameObjects(frameInfo);
                }
                else
                {
                    renderSystem->renderGameObjects(frameInfo);
                }

                pointLightSystem->render(frameInfo);

                offscreenRenderer->endOffscreenRenderPass(commandBuffer);

                offscreenRenderer->createNextImage();
                offscreenImageDescriptor = ImGui_ImplVulkan_AddTexture(
                    offscreenRenderer->getImageSampler(),
                    offscreenRenderer->getCurrentImageView(),
                    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);

                frameInfo.currentFrameSet = offscreenImageDescriptor;

                // render
                interfaceSystem->newFrame(frameInfo);

                renderer->startSwapChainRenderPass(commandBuffer);

                /* if (ImGui::Begin("Image", nullptr))
                {
                    ImGui::Image((ImTextureID)skyboxSet, ImVec2(500,500));
                    ImGui::End();
                } */

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
        obj.textureDescriptorSet = objectInfo.texture;
        obj.transform.translation = objectInfo.position;
        obj.transform.scale = objectInfo.scale;

        gameObjects.emplace(obj.getId(), std::move(obj));
    }

    void MasterRenderSystem::loadGameObjects()
    {
        std::shared_ptr<GWModel>
            Model;

        CubeMapInfo info{};
        info.negX = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/negx.jpg";
        info.posX = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/posx.jpg";
        info.negY = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/negy.jpg";
        info.posY = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/posy.jpg"; 
        info.negZ = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/negz.jpg";
        info.posZ = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/posz.jpg";
        VkDescriptorSet skyboxSet;
        CubeMap cubeMap = cubemapHandler->createCubeMap(info);
        Texture texture2{};
        texture2.textureImage = cubeMap.Cubeimage;
        textureHandler->createSampler(0, texture2.textureSampler);

        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/cube.obj", Model, false);

        GWGameObject skyboxObject = GWGameObject::createGameObject("Skybox");
        skyboxObject.model = Model;
        skyboxObject.transform.scale = 85.f;

        gameObjects.emplace(skyboxObject.getId(), std::move(skyboxObject));

        createSet(skyboxSet, texture2);
        skyboxSystem->setSkybox(skyboxSet);

        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/viking_room.obj", Model, false);

        auto model = GWGameObject::createGameObject("Viking Room");
        model.model = Model;
        model.transform.translation = {0.f, .5f, 1.f};
        model.transform.rotation.x = .25f * glm::two_pi<float>();
        model.transform.scale = 1.f;

        std::string pathToTexture = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/viking_room.png";
        Texture texture = textureHandler->createTexture(pathToTexture);

        createSet(model.textureDescriptorSet, texture);
        gameObjects.emplace(model.getId(), std::move(model));

        std::vector<glm::vec3> lightColors{
            {1.f, 1.f, 1.f}
        };
    }
}
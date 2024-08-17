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
        materialHandler = std::make_unique<GWMaterialHandler>(device);

        initialize();
        loadGameObjects();

        //Initializes GUI
        interfaceSystem = std::make_unique<GWInterface>(window, device, renderer->getSwapChainImageFormat(), textureHandler, materialHandler);
        interfaceSystem->setLoadGameObjectCallback([this](GameObjectInfo& objectInfo) {
            currentScene.createGameObject(objectInfo);
        });

        interfaceSystem->setCreateTextureCallback([this](VkDescriptorSet& set, Texture& texture) {
            createSet(set, texture);
        });

        currentScene.createCamera();
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

    void MasterRenderSystem::updateCamera(FrameInfo& frameInfo)
    {
        auto& viewerObject = frameInfo.gameObjects.at(frameInfo.currentCamera.getViewerObject());
        cameraController.moveInPlaneXZ(window.getWindow(), frameInfo.deltaTime,  viewerObject);
        frameInfo.currentCamera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = renderer->getAspectRatio();
        frameInfo.currentCamera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
    }

    void MasterRenderSystem::run()
    {
        currentTime = std::chrono::high_resolution_clock::now();

        while (!window.shouldClose())
        {
            glfwPollEvents();
            auto newTime = std::chrono::high_resolution_clock::now();
            float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            if (auto commandBuffer = renderer->startFrame())
            {
                int frameIndex = renderer->getFrameIndex();

                bool isWireFrame = false;

                // update
                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    currentScene.getCurrentCamera(),
                    globalDescriptorSets[frameIndex],
                    currentScene.getSceneInfo().gameObjects,
                    VK_NULL_HANDLE};

                updateCamera(frameInfo);

                GlobalUbo ubo{};
                ubo.projection = frameInfo.currentCamera.getProjection();
                ubo.view = frameInfo.currentCamera.getView();
                ubo.inverseView = frameInfo.currentCamera.getInverseView();
                ubo.sunLight = interfaceSystem->getLightDirection(frameInfo.gameObjects.at(1));
                pointLightSystem->update(frameInfo, ubo);
                materialHandler->setMaterials(ubo);
                globalUboBuffer->writeToIndex(&ubo, frameIndex);
                globalUboBuffer->flushIndex(frameIndex);

                if (glfwGetKey(window.getWindow(), cameraController.keys.activateWireframe))
                {
                    isWireFrame = true;
                }

                offscreenRenderer->startOffscreenRenderPass(commandBuffer);

                skyboxSystem->render(frameInfo);

                if (isWireFrame)
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
                frameInfo.currentFrameSet = ImGui_ImplVulkan_AddTexture(
                    offscreenRenderer->getImageSampler(),
                    offscreenRenderer->getCurrentImageView(),
                    VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL);

                // render
                interfaceSystem->newFrame(frameInfo);

                renderer->startSwapChainRenderPass(commandBuffer);

                interfaceSystem->render(commandBuffer);
                renderer->endSwapChainRenderPass(commandBuffer);
                renderer->endFrame();

                ImGui_ImplVulkan_RemoveTexture(frameInfo.currentFrameSet);
                frameInfo.currentFrameSet = VK_NULL_HANDLE;

                isWireFrame = false;
            }
        }
    }

    void MasterRenderSystem::loadGameObjects()
    {
        std::shared_ptr<GWModel>
            Model;

        CubeMapInfo info{};
        info.negX = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/nx.png";
        info.posX = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/px.png";
        info.negY = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/ny.png";
        info.posY = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/py.png"; 
        info.negZ = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/nz.png";
        info.posZ = "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/cubeMap/pz.png";
        VkDescriptorSet skyboxSet;
        CubeMap cubeMap = cubemapHandler->createCubeMap(info);
        Texture texture2{};
        texture2.textureImage = cubeMap.Cubeimage;
        textureHandler->createSampler(0, texture2.textureSampler);

        createSet(skyboxSet, texture2);
        skyboxSystem->setSkybox(skyboxSet);

        Texture no_texture = textureHandler->createTexture(std::string("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/no_texture.png"), true);

        VkDescriptorSet no_texture_set;

        createSet(no_texture_set, no_texture);

        modelLoader.importFile("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/sphere.obj", Model, false);

        int index = 0;
        for (uint32_t x = 0; x < 5; x++)
        {
            for (uint32_t z = 0; z < 5; z++)
            {
                index++;
                auto sphere = GWGameObject::createGameObject("Sphere " + std::to_string(index));
                sphere.model = Model;
                sphere.textureDescriptorSet = no_texture_set;
                sphere.transform.translation.x = x * 1.25;
                sphere.transform.translation.z = z * 1.25;
                sphere.transform.scale = .5f;

                sphere.Material = materialHandler->createMaterial(
                    1.0f / (x + 1), 
                    1.0f / (z + 1), 
                    { 
                        (1.0f + cos(3.14159f * x / 5.0f)) * 0.5f,  // R component varies between 0 and 1
                        (1.0f + cos(3.14159f * z / 5.0f)) * 0.5f,  // G component varies between 0 and 1
                        (1.0f + sin(3.14159f * (x + z) / 10.0f)) * 0.5f, // B component varies between 0 and 1
                        1.0f
                    }
                );
                
                currentScene.createGameObject(sphere);
            }
        }
    }
}
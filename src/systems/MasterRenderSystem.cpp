#include "MasterRenderSystem.hpp"

#include "../GWBuffer.hpp"

#include <numeric>
#include <iostream>
#include <chrono>
#include <iostream>
#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>
#include <optional>

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

        interfaceSystem->setCreateTextureCallback([this](VkDescriptorSet& set, Texture& texture, bool replace = false) {
            currentScene->createSet(set, texture, replace);
        });

        interfaceSystem->setSaveSceneCallback([this](const std::string path) {
            currentScene->saveScene(path);
        });

        interfaceSystem->setLoadSceneCallback([this](const std::string path) {
            loadNewScene(path);
        });

        interfaceSystem->setCreateMeshCallback([this](const std::string path, std::optional<uint32_t> replaceId = std::nullopt)
        {
            return currentScene->createMesh(path, replaceId);
        });

        interfaceSystem->setRemoveMeshCallback([this](uint32_t id)
        {
            currentScene->removeMesh(id);
        });

        interfaceSystem->setCreateObjectCallback([this](GameObjectType type)
        {
            currentScene->createGameObject(type);
        });

        interfaceSystem->setDeleteObjectCallback([this](uint32_t id)
                                                 { currentScene->removeGameObject(id); });

        currentScene->createCamera();
    }

    void MasterRenderSystem::initialize()
    {
        globalPool = GWDescriptorPool::Builder(device)
                         .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                         .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
                         .build();

        auto globalSetLayout = GWDescriptorSetLayout::Builder(device)
                                   .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                   .build();

        texturePool = GWDescriptorPool::Builder(device)
                          .setMaxSets(1000)
                          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
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

        SceneCreateInfo createInfo{device, textureSetLayout, texturePool, modelLoader, jsonHandler, textureHandler, materialHandler};
        
        currentScene = std::make_unique<GWScene>(createInfo);

        renderSystem = std::make_unique<RenderSystem>(device, offscreenRenderer->getRenderPass(), false, setLayouts);
        wireframeRenderSystem = std::make_unique<RenderSystem>(device, offscreenRenderer->getRenderPass(), true, setLayouts);
        pointLightSystem = std::make_unique<PointLightSystem>(device, offscreenRenderer->getRenderPass(), globalSetLayout->getDescriptorSetLayout());
        skyboxSystem = std::make_unique<SkyboxSystem>(device, offscreenRenderer->getRenderPass(), setLayouts);
    }

    void MasterRenderSystem::updateCamera(FrameInfo& frameInfo)
    {
        auto& camera = frameInfo.currentInfo.currentCamera;
        auto &viewerObject = frameInfo.currentInfo.gameObjects.at(camera.getViewerObject());
        cameraController.moveInPlaneXZ(window.getWindow(), frameInfo.deltaTime,  viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.getRotation());

        float aspect = renderer->getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

        if (frameInfo.flags.frustumCulling)
            camera.updateFrustumPlanes();
    }

    void MasterRenderSystem::loadNewScene(const std::string pathToFile)
    {
        std::ifstream file(pathToFile);

        std::string json;

        if (file.is_open())
        {
            try {
                json = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

                SceneCreateInfo createInfo{device, textureSetLayout, texturePool, modelLoader, jsonHandler, textureHandler, materialHandler, "DefaultScene", json};

                vkDeviceWaitIdle(device.device());

                currentScene->loadScene(json);
                isLoading = true;
            }
            catch (const nlohmann::json::parse_error &e)
            {
                std::cerr << "Parse error: " << e.what() << std::endl;
            }
        } else {
            std::cout << "Cannot open file! " << pathToFile << std::endl;
        }
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

                auto& interfaceFlags = interfaceSystem->getFlags();

                SceneInfo currentInfo{
                    currentScene->getCurrentCamera(),
                    currentScene->getGameObjects(),
                    currentScene->getMeshes(),
                    currentScene->getTextures()};

                FrameInfo frameInfo{
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    globalDescriptorSets[frameIndex],
                    currentInfo,
                    VK_NULL_HANDLE};

                frameInfo.flags.frustumCulling = interfaceFlags.frustumCulling;

                updateCamera(frameInfo);

                GlobalUbo ubo{};
                ubo.projection = frameInfo.currentInfo.currentCamera.getProjection();
                ubo.view = frameInfo.currentInfo.currentCamera.getView();
                ubo.inverseView = frameInfo.currentInfo.currentCamera.getInverseView();
                ubo.sunLight = interfaceSystem->getLightDirection(frameInfo.currentInfo.gameObjects.at(1));
                pointLightSystem->update(frameInfo, ubo);
                materialHandler->setMaterials(ubo);
                globalUboBuffer->writeToIndex(&ubo, frameIndex);
                globalUboBuffer->flushIndex(frameIndex);

                if (glfwGetKey(window.getWindow(), cameraController.keys.activateWireframe))
                {
                    isWireFrame = true;
                }

                offscreenRenderer->startOffscreenRenderPass(commandBuffer);

                if (!isLoading)
                {
                    skyboxSystem->render(frameInfo);
                }

                if (isWireFrame)
                {
                    wireframeRenderSystem->renderGameObjects(frameInfo);
                }
                else
                {
                    renderSystem->renderGameObjects(frameInfo);
                }

                pointLightSystem->render(frameInfo);

                if (isLoading)
                {
                    skyboxSystem->render(frameInfo);
                    isLoading = false;
                }

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

        Texture no_texture = textureHandler->createTexture(std::string("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/textures/no_texture.png"), true);

        VkDescriptorSet no_texture_set;

        currentScene->createSet(no_texture_set, no_texture);

        currentScene->createSet(skyboxSet, texture2);
        skyboxSystem->setSkybox(skyboxSet, texture2.id);

        uint32_t model = currentScene->createMesh("C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/models/sphere.obj", std::nullopt);

        int index = 0;
        for (uint32_t x = 0; x < 3; x++)
        {
            for (uint32_t z = 0; z < 3; z++)
            {
                index++;
                auto sphere = GWGameObject::createGameObject("Sphere " + std::to_string(index));
                sphere.model = model;
                sphere.Textures[0] = 0;
                sphere.transform.translation.x = x * 1.25;
                sphere.transform.translation.z = z * 1.25;
                sphere.transform.scale = glm::vec3{ .5f };

                sphere.Material = materialHandler->createMaterial(
                    1.0f / (x + 1), 
                    1.0f / (z + 1), 
                    { 
                        (1.0f + cos(3.14159f * x / 5.0f)) * 0.5f,  // R component varies between 0 and 1
                        (1.0f + cos(3.14159f * z / 5.0f)) * 0.5f,  // G component varies between 0 and 1
                        (1.0f + sin(3.14159f * (x + z) / 10.0f)) * 0.5f, // B component varies between 0 and 1
                        1.0f
                    }, std::string("Material " + std::to_string(index))
                );
                
                currentScene->createGameObject(sphere);
            }
        }
    }
}
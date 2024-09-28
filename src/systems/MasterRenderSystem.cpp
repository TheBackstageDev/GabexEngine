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
        shadowMapRenderer = std::make_unique<GWShadowRenderer>(window, device, renderer->getSwapChainDepthFormat(), renderer->getImageCount());
        cubemapHandler = std::make_unique<GWCubemapHandler>(device);
        materialHandler = std::make_unique<GWMaterialHandler>(device);

        initialize();
        loadGameObjects();

        //Initializes GUI
        interfaceSystem = std::make_unique<GWInterface>(window, device, renderer->getSwapChainImageFormat(), textureHandler, materialHandler);

        interfaceSystem->setCreateTextureCallback([this](Texture& texture, bool replace = false) {
            currentScene->createSet(texture, replace);
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
                          .setMaxSets(2)
                          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, device.properties.limits.maxPerStageDescriptorSampledImages)
                          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT)
                          .build();

        textureSetLayout = GWDescriptorSetLayout::Builder(device)
                               .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, device.properties.limits.maxPerStageDescriptorSamplers)
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

        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = globalUboBuffer->descriptorInfo();
            GWDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        std::vector<VkDescriptorSetLayout> setLayouts = {globalSetLayout->getDescriptorSetLayout(), textureSetLayout->getDescriptorSetLayout()};

        textureHandler = std::make_unique<GWTextureHandler>(imageLoader, device);

        modelLoader.setCreateTextureCallback([this](Texture &texture, bool replace = false)
                                             { currentScene->createSet(texture, replace); });

        SceneCreateInfo createInfo{device, textureSetLayout, texturePool, modelLoader, jsonHandler, textureHandler, materialHandler};
        
        currentScene = std::make_unique<GWScene>(createInfo);

        renderSystem = std::make_unique<RenderSystem>(device, false, setLayouts);
        wireframeRenderSystem = std::make_unique<RenderSystem>(device, true, setLayouts);
        lightSystem = std::make_unique<LightSystem>();
        skyboxSystem = std::make_unique<SkyboxSystem>(device, setLayouts);
        shadowSystem = std::make_unique<ShadowSystem>(device, setLayouts);
    }

    void MasterRenderSystem::updateCamera(FrameInfo& frameInfo)
    {
        auto& camera = frameInfo.currentInfo.currentCamera;
        auto &viewerObject = frameInfo.currentInfo.gameObjects.at(camera.getViewerObject());
        cameraController.moveInPlaneXZ(window.getWindow(), frameInfo.deltaTime,  viewerObject);
        camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.getRotation());

        float aspect = renderer->getAspectRatio();
        camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
        //camera.setOrthographicProjection(-aspect, aspect, -1, 1, 0.1f, 100.f);

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
                auto& currentCam = currentScene->getCurrentCamera();

                SceneInfo currentInfo{
                    currentCam,
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
                ubo.sunLightSpaceMatrix = lightSystem->calculateDirectionalLightMatrix(10.f, ubo.sunLight);
                lightSystem->update(frameInfo, ubo);
                materialHandler->setMaterials(ubo);
                globalUboBuffer->writeToIndex(&ubo, frameIndex);
                globalUboBuffer->flushIndex(frameIndex);

                if (glfwGetKey(window.getWindow(), cameraController.keys.activateWireframe))
                {
                    isWireFrame = true;
                }

                shadowMapRenderer->startOffscreenRenderPass(commandBuffer);
                shadowSystem->render(frameInfo);
                shadowMapRenderer->endOffscreenRenderPass(commandBuffer);

                shadowMapRenderer->createNextImage();
                auto shadowImageView = shadowMapRenderer->getCurrentImageView();
                auto shadowSampler = shadowMapRenderer->getImageSampler();

                currentScene->createSet(VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, shadowImageView, shadowSampler, 0);

                offscreenRenderer->startOffscreenRenderPass(commandBuffer);

                if (!isLoading)
                {
                    skyboxSystem->render(frameInfo);
                }

                if (isWireFrame)
                {
                    wireframeRenderSystem->renderGameObjects(frameInfo);
                } else {
                   renderSystem->renderGameObjects(frameInfo);
                }

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

                vkDeviceWaitIdle(device.device());
                ImGui_ImplVulkan_RemoveTexture(frameInfo.currentFrameSet);
                frameInfo.currentFrameSet = VK_NULL_HANDLE;

                isWireFrame = false;
            }
        }

        vkDeviceWaitIdle(device.device());
    }

    void MasterRenderSystem::loadGameObjects()
    {
        Texture no_texture = textureHandler->createTexture(std::string("src/textures/no_texture.png"), true);
        currentScene->createSet(no_texture);
        CubeMapInfo info{};
        info.negX = "src/textures/cubeMap/nx.png";
        info.posX = "src/textures/cubeMap/px.png";
        info.negY = "src/textures/cubeMap/ny.png";
        info.posY = "src/textures/cubeMap/py.png";
        info.negZ = "src/textures/cubeMap/nz.png";
        info.posZ = "src/textures/cubeMap/pz.png";
        CubeMap cubeMap = cubemapHandler->createCubeMap(info);
        Texture texture2{};
        texture2.textureImage = cubeMap.Cubeimage;
        GWIN::createSampler(device, texture2.textureSampler, 1);

        VkDescriptorSet skyboxSet = currentScene->retcreateSet(
        texture2.textureImage.layout, 
        texture2.textureImage.imageView,
        texture2.textureSampler, 1);

        skyboxSystem->setSkybox(skyboxSet, texture2.id);

        uint32_t model = currentScene->createMesh("src/models/Sponza/sponza.obj", std::nullopt);
        GWGameObject& obj = GWGameObject::createGameObject("Sponza");
        obj.model = model;
        obj.transform.scale = glm::vec3{0.02f, 0.02f, 0.02f};

        currentScene->createGameObject(obj);

        uint32_t model2 = currentScene->createMesh("src/models/quad.obj", std::nullopt);
        GWGameObject& obj2 = GWGameObject::createGameObject("Quad");
        obj2.model = model2;

        currentScene->getMeshes().at(model2)->Textures[0] = 0;

        currentScene->createGameObject(obj2);
    }
}
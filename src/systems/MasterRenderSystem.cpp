#include "MasterRenderSystem.hpp"

#include "../GWBuffer.hpp"
#include "../EC/GWModelLoader.hpp"

#include <numeric>
#include <iostream>
#include <chrono>

#include <glm/gtc/constants.hpp>
#include <glm/glm.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

namespace GWIN
{
    static void check_vk_result(VkResult err)
    {
        if (err == 0)
            return;
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
            abort();
    }

    MasterRenderSystem::MasterRenderSystem(GWindow& window, GWinDevice& device)
        : window(window), device(device)
    {
        renderer = std::make_unique<GWRenderer>(window, device);
        loadGameObjects();
        initialize();
        //initializeGui();
    }

    /*void MasterRenderSystem::initializeGui()
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        // Setup Dear ImGui context
        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForVulkan(window.getWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = device.getInstance();
        init_info.PhysicalDevice = device.phyDevice();
        init_info.Device = device.device();
        init_info.Queue = device.graphicsQueue();
        init_info.DescriptorPool = &imguipool;
        init_info.RenderPass = renderer->getRenderPass();
        init_info.MinImageCount = GWinSwapChain::MAX_FRAMES_IN_FLIGHT;
        init_info.ImageCount = renderer->getImageCount();
        init_info.UseDynamicRendering = true;
        init_info.CheckVkResultFn = check_vk_result;
        ImGui_ImplVulkan_Init(&init_info);
    }*/

    void MasterRenderSystem::initialize()
    {
        globalPool = GWDescriptorPool::Builder(device)
                    .setMaxSets(GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GWinSwapChain::MAX_FRAMES_IN_FLIGHT)
                    .build();
                         
        auto minOffsetAlignment = std::lcm(
            device.properties.limits.minUniformBufferOffsetAlignment,
            device.properties.limits.nonCoherentAtomSize);

        globalUboBuffer = std::make_unique<GWBuffer>(  
            device, 
            sizeof(GlobalUbo), 
            GWinSwapChain::MAX_FRAMES_IN_FLIGHT, 
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            minOffsetAlignment
        );

        globalUboBuffer->map();

        auto globalSetLayout = GWDescriptorSetLayout::Builder(device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
                                .build();
        
        globalDescriptorSets.resize(GWinSwapChain::MAX_FRAMES_IN_FLIGHT);

        for (int i = 0; i < globalDescriptorSets.size(); ++i)
        {
            auto bufferInfo = globalUboBuffer->descriptorInfo();
            GWDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
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
        auto viewerObject = GWGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5;
        
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
                FrameInfo frameInfo
                {
                    frameIndex,
                    deltaTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex],
                    gameObjects,
                    false
                };
                
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

                // render
                renderer->startSwapChainRenderPass(commandBuffer);

                if (frameInfo.isWireFrame)
                {
                    wireframeRenderSystem->renderGameObjects(frameInfo);
                }
                else
                {
                    renderSystem->renderGameObjects(frameInfo);
                }

                pointLightSystem->render(frameInfo);

                renderer->endSwapChainRenderPass(commandBuffer);
                renderer->endFrame();

                frameInfo.isWireFrame = false;
            }
        }
    }

    void MasterRenderSystem::loadGameObjects()
    {
        GWModelLoader modelLoader{device};
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
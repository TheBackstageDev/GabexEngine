#pragma once

#include "../EC/GWRenderer.hpp"
#include "../EC/GWDescriptors.hpp"
#include "../EC/keyboard_movement_controller.hpp"
#include "RenderSystem.hpp"
#include "lightSystem.hpp"
#include "ShadowSystem.hpp"
#include "InterfaceSystem.hpp"
#include "GWTextureHandler.hpp"
#include "GWMaterialHandler.hpp"
#include "SkyboxSystem.hpp"
#include "GWScene.hpp"

#include "../GWRendererToolkit.hpp"
#include "GWOffscreenRenderer.hpp"
#include "GWShadowRenderer.hpp"

#include <stdexcept>
#include <chrono>

namespace GWIN
{
    class MasterRenderSystem
    {
    public:
        MasterRenderSystem(GWindow& window, GWinDevice& device);
        //~MasterRenderSystem();

        MasterRenderSystem(const MasterRenderSystem&) = delete;
        MasterRenderSystem& operator=(const MasterRenderSystem&) = delete;

        void run();

    private:
        void initialize();
        void updateCamera(FrameInfo &frameInfo, float FOV);
        void loadGameObjects();

        void loadNewScene(const std::string pathToFile);

        GWindow& window;
        GWinDevice& device;

        std::unique_ptr<GWRenderer> renderer;
        std::unique_ptr<GWOffscreenRenderer> offscreenRenderer;
        std::unique_ptr<GWShadowRenderer> shadowMapRenderer;

        //Render Systems
        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<RenderSystem> wireframeRenderSystem;
        std::unique_ptr<LightSystem> lightSystem;
        std::unique_ptr<SkyboxSystem> skyboxSystem;
        std::unique_ptr<GWInterface> interfaceSystem;
        std::unique_ptr<ShadowSystem> shadowSystem;

        std::unique_ptr<GWBuffer> globalUboBuffer;
        std::unique_ptr<GWBuffer> lightBuffer;
        std::unique_ptr<GWBuffer> materialBuffer;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        std::unique_ptr<GWDescriptorPool> globalPool{};
        std::unique_ptr<GWDescriptorPool> texturePool{};
        std::unique_ptr<GWDescriptorSetLayout> textureSetLayout;

        GWImageLoader imageLoader{device};
        std::unique_ptr<GWTextureHandler> textureHandler;
        std::unique_ptr<GWCubemapHandler> cubemapHandler;
        std::unique_ptr<GWMaterialHandler> materialHandler;
        
        GWModelLoader modelLoader{device, textureHandler};

        JSONHandler jsonHandler{};

        std::unique_ptr<GWScene> currentScene;

        keyboardMovementController cameraController{};

        bool isLoading{false};

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
    };
}
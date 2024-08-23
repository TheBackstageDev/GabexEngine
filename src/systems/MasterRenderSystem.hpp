#pragma once

#include "../EC/GWGameObject.hpp"
#include "../EC/GWRenderer.hpp"
#include "../EC/GWDescriptors.hpp"
#include "../EC/GWFrameInfo.hpp"
#include "../EC/keyboard_movement_controller.hpp"
#include "../EC/GWModelLoader.hpp"
#include "RenderSystem.hpp"
#include "PointLightSystem.hpp"
#include "InterfaceSystem.hpp"
#include "GWTextureHandler.hpp"
#include "GWMaterialHandler.hpp"
#include "SkyboxSystem.hpp"
#include "GWScene.hpp"

#include "GWOffscreenRenderer.hpp"

#include <memory>
#include <vector>
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
        void updateCamera(FrameInfo &frameInfo);
        void loadGameObjects();

        void loadNewScene(const std::string pathToFile);

        void MasterRenderSystem::createSet(VkDescriptorSet &set, Texture &texture);

        GWindow& window;
        GWinDevice& device;

        std::unique_ptr<GWRenderer> renderer;
        std::unique_ptr<GWOffscreenRenderer> offscreenRenderer;

        //Render Systems
        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<RenderSystem> wireframeRenderSystem;
        std::unique_ptr<PointLightSystem> pointLightSystem;
        std::unique_ptr<SkyboxSystem> skyboxSystem;
        std::unique_ptr<GWInterface> interfaceSystem;

        std::unique_ptr<GWBuffer> globalUboBuffer;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        std::unique_ptr<GWDescriptorPool> globalPool{};
        std::unique_ptr<GWDescriptorPool> texturePool{};
        std::unique_ptr<GWDescriptorSetLayout> textureSetLayout;

        GWModelLoader modelLoader{device};
        GWImageLoader imageLoader{device};
        std::unique_ptr<GWTextureHandler> textureHandler;
        std::unique_ptr<GWCubemapHandler> cubemapHandler;
        std::unique_ptr<GWMaterialHandler> materialHandler;

        JSONHandler jsonHandler{};

        std::unique_ptr<GWScene> currentScene;

        keyboardMovementController cameraController{};

        bool isLoading{false};

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
    };
}
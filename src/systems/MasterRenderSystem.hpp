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
        void initializeGui();
        void updateCamera(GWGameObject& viewerObject, float deltaTime);
        void loadGameObjects();
        
        GWindow& window;
        GWinDevice& device;
        std::unique_ptr<GWRenderer> renderer;
        
        std::unique_ptr<GWDescriptorPool> globalPool{};

        //Render Systems
        std::unique_ptr<RenderSystem> renderSystem;
        std::unique_ptr<RenderSystem> wireframeRenderSystem;
        std::unique_ptr<PointLightSystem> pointLightSystem;
        std::unique_ptr<GWInterface> interfaceSystem;

        GWModelLoader modelLoader{device};
        
        GWGameObject::map gameObjects;
        GWCamera camera{};
        keyboardMovementController cameraController{};
        std::vector<VkDescriptorSet> globalDescriptorSets;
        std::unique_ptr<GWBuffer> globalUboBuffer;

        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime;
    };
}
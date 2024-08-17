#pragma once

#include "../GWindow.hpp"
#include "../GWDevice.hpp"
#include "../GWSwapChain.hpp"
#include "../EC/GWDescriptors.hpp"
#include "../EC/GWFrameInfo.hpp"
#include "../EC/GWTextureHandler.hpp"
#include "../EC/GWMaterialHandler.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_vulkan.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include <functional>
#define USE_THUMBNAILS
#include "imguifiledialog/ImGuiFileDialog.h"

#include <imguizmo/ImGuizmo.h>

// pre-made interfaces
#include "./interface/Console.hpp"
#include "./interface/ObjectList.hpp"
#include "./interface/Assets.hpp"

namespace GWIN
{
    class GWInterface
    {
    public:
        GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat, std::unique_ptr<GWTextureHandler> &textureHandler,
                    std::unique_ptr<GWMaterialHandler>& materialHandler);
        ~GWInterface();

        void newFrame(FrameInfo& frameInfo);
        void render(VkCommandBuffer commandBuffer);

        void setLoadGameObjectCallback(std::function<void(GameObjectInfo& objectInfo)> callback);
        void setCreateTextureCallback(std::function<void(VkDescriptorSet &, Texture &texture)> callback);

        GWConsole getConsole() const { return console; }

        glm::vec4 getLightDirection(GWGameObject& directionalLight) { return {directionalLight.transform.rotation, DirectionalLightingIntensity}; }

    private:
        GWindow& window;
        GWinDevice& device;

        std::function<void(GameObjectInfo &objectInfo)> loadGameObjectCallback;
        std::function<void(VkDescriptorSet& set, Texture& texture)> createTextureCallback;

        std::unique_ptr<GWTextureHandler>& textureHandler;
        std::unique_ptr<GWMaterialHandler>& materialHandler;

        void initializeGUI(VkFormat imageFormat);
        void drawImGuizmo(FrameInfo &frameInfo, ImDrawList* drawList);
        void drawSceneSettings();

        GWObjectList objectList{};
        GWConsole console{};
        std::unique_ptr<AssetsWindow> assets;
        void drawFileDialog();

        bool showCreateObjectWindow{false};
        ImGuizmo::OPERATION mCurrentGizmoOperation{ImGuizmo::TRANSLATE};

        //values
        float DirectionalLightingIntensity = 1.f;

        //Flags
        bool showShadows{true};

        std::unique_ptr<GWDescriptorPool> guipool;
    };
}
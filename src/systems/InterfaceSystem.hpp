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
#include "./interface/DebugVisuals.hpp"

// pre-made interfaces
#include "./interface/Console.hpp"
#include "./interface/ObjectList.hpp"
#include "./interface/Assets.hpp"

namespace GWIN
{
    struct Flags
    {
        bool showShadows{true};
        bool frustumCulling{false};
        bool debugElements{true};
        bool debugHandles{true};
    };

    class GWInterface
    {
    public:
        GWInterface(GWindow &window, GWinDevice &device, VkFormat imageFormat, std::unique_ptr<GWTextureHandler> &textureHandler,
                    std::unique_ptr<GWMaterialHandler>& materialHandler);
        ~GWInterface();

        void newFrame(FrameInfo& frameInfo);
        void render(VkCommandBuffer commandBuffer);

        void setCreateTextureCallback(std::function<void(Texture &texture, uint32_t id)> callback) { createTextureCallback = callback; objectList.setCreateTextureCallback(callback); };
        void setSaveSceneCallback(std::function<void(const std::string path)> callback) { SaveSceneCallback = callback;  };
        void setLoadSceneCallback(std::function<void(const std::string path)> callback) { LoadSceneCallback = callback; };
        void setCreateMeshCallback(std::function<uint32_t(const std::string path, std::optional<uint32_t> replaceId)> callback) { createMeshCallback = callback; objectList.setCreateMeshCallback(callback); };
        void setRemoveMeshCallback(std::function<void(uint32_t id)> callback) { removeMeshCallback = callback; objectList.setRemoveMeshCallback(callback); };

        void setCreateObjectCallback(std::function<void(GameObjectType type)> callback) { createObjectCallback = callback; objectList.setCreateObjectCallback(callback); };
        void setDeleteObjectCallback(std::function<void(uint32_t id)> callback) { removeObjectCallback = callback; objectList.setDeleteObjectCallback(callback); };

        GWConsole getConsole() const { return console; }

        float getExposure() { return exposure; }
        glm::vec4 getLightDirection(GWGameObject& directionalLight) { return {directionalLight.transform.getRotation(), DirectionalLightingIntensity}; }

        Flags getFlags() { return flags; }

    private:
        GWindow& window;
        GWinDevice& device;

        std::function<void(Texture &texture, bool replace)> createTextureCallback;
        std::function<void(const std::string path)> SaveSceneCallback;
        std::function<void(const std::string path)> LoadSceneCallback;
        std::function<uint32_t(const std::string path, std::optional<uint32_t> replaceId)> createMeshCallback;
        std::function<void(uint32_t id)> removeMeshCallback;
        std::function<void(GameObjectType type)> createObjectCallback;
        std::function<void(uint32_t id)> removeObjectCallback;

        std::unique_ptr<GWTextureHandler>& textureHandler;
        std::unique_ptr<GWMaterialHandler>& materialHandler;

        void initializeGUI(VkFormat imageFormat);
        void drawImGuizmo(FrameInfo &frameInfo, ImDrawList* drawList);
        void drawSceneSettings();

        GWConsole console{};
        std::unique_ptr<AssetsWindow> assets;
        GWObjectList objectList{materialHandler, assets};
        void drawFileDialog();

        ImGuizmo::OPERATION mCurrentGizmoOperation{ImGuizmo::TRANSLATE};
        DebugVisuals debugVisuals{};

        //values
        float DirectionalLightingIntensity = 1.f;
        float exposure = 1.f;

        Flags flags; 

        std::unique_ptr<GWDescriptorPool> guipool;
    };
}
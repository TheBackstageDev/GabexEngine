#pragma once

#include <imgui/imgui.h>
#include "GWFrameInfo.hpp"
#include "./Assets.hpp"
#include "GWMaterialHandler.hpp"
#include <imguifiledialog/ImGuiFileDialog.h>

#include <vector>
#include <string>
#include <optional>

namespace GWIN
{
    class GWObjectList
    {
    public:
        GWObjectList(std::unique_ptr<GWMaterialHandler> &materialHandler, std::unique_ptr<AssetsWindow>& assets) : materialHandler(materialHandler), assets(assets) {};
        void Draw(FrameInfo& frameInfo);

        uint32_t getSelectedObject() { return selectedItem; }
        bool isAssetSelected() { return AssetSelected; }

        void setCreateTextureCallback(std::function<void(Texture &texture, uint32_t id)> callback) { createTextureCallback = callback; };
        void setCreateMeshCallback(std::function<uint32_t(const std::string path, std::optional<uint32_t> replaceId)> callback) { createMeshCallback = callback; };
        void setRemoveMeshCallback(std::function<void(uint32_t id)> callback) { removeMeshCallback = callback; };
        void setCreateObjectCallback(std::function<void(GameObjectType type)> callback) { createObjectCallback = callback; };
        void setDeleteObjectCallback(std::function<void(uint32_t id)> callback) { removeObjectCallback = callback; };


    private:
        std::unique_ptr<GWMaterialHandler> &materialHandler;

        int selectedItem{-1}; //-1: none selected;
        bool isEditingName{false};
        bool rotationChanged{false};
        bool AssetSelected{false};

        //Value Buffers
        char nameBuffer[1000]{""};
        glm::vec3 positionBuffer{0.0f, 0.0f, 0.0f};
        glm::vec3 rotationBuffer{0.0f, 0.0f, 0.0f};
        glm::vec3 scaleBuffer = { 1.0f, 1.0f, 1.0f };

        std::function<uint32_t(const std::string path, std::optional<uint32_t> replaceId)> createMeshCallback;
        std::function<void(Texture &texture, uint32_t id)> createTextureCallback; 
        std::function<void(uint32_t id)> removeMeshCallback;
        std::function<void(GameObjectType type)> createObjectCallback;
        std::function<void(uint32_t id)> removeObjectCallback;

        void assetList(FrameInfo &frameInfo);

        std::unique_ptr<AssetsWindow>& assets;

        void inputModel(GWGameObject &selectedObject);
        void inputTexture(std::shared_ptr<GWModel> &selectedObject);
        void inputMaterial(std::shared_ptr<GWModel> &selectedObject);

        void inputLight(GWGameObject &selectedObject); 
        void inputRotation(GWGameObject &selectedObject);
        void inputPosition(GWGameObject &selectedObject);
        void inspectorGuis(GWGameObject &selectedObject, FrameInfo &frameInfo);

        //Asset-Type specific options
        void materialEditor(Asset& selectedAsset);
        void meshEditor(Asset &selectedAsset, FrameInfo &frameInfo);
        void textureEditor(Asset& selectedAsset, FrameInfo& frameInfo);

        void createComponentAsset(const Asset& selectedAsset, FrameInfo& frameInfo);
        void addComponent(FrameInfo& frameInfo);
    };
}
#pragma once

#include <imgui/imgui.h>
#include "GWFrameInfo.hpp"
#include "./Assets.hpp"
#include "GWMaterialHandler.hpp"
#include <imguifiledialog/ImGuiFileDialog.h>

#include <vector>
#include <string>

namespace GWIN
{
    class GWObjectList
    {
    public:
        GWObjectList(std::unique_ptr<GWMaterialHandler> &materialHandler, std::unique_ptr<AssetsWindow>& assets) : materialHandler(materialHandler), assets(assets) {};
        void Draw(FrameInfo& frameInfo);

        uint32_t getSelectedObject() { return selectedItem; }
        bool isAssetSelected() { return AssetSelected; }

    private:
        std::unique_ptr<GWMaterialHandler> &materialHandler;

        int selectedItem{-1}; //-1: none selected;
        int selectedMaterial = {0};
        bool isEditingName{false};
        bool rotationChanged{false};
        bool AssetSelected{false};

        //Value Buffers
        char nameBuffer[1000]{""};
        glm::vec3 positionBuffer{0.0f, 0.0f, 0.0f};
        glm::vec3 rotationBuffer{0.0f, 0.0f, 0.0f};
        float scaleBuffer = 1.f;

        void assetList(FrameInfo &frameInfo);

        std::unique_ptr<AssetsWindow>& assets;

        void inputRotation(GWGameObject &selectedObject);
        void inputPosition(GWGameObject &selectedObject);
        void inspectorGuis(GWGameObject &selectedObject);

        //Asset-Type specific options
        void materialEditor(Asset& selectedAsset);
        void meshEditor(Asset &selectedAsset, FrameInfo &frameInfo);
        void textureEditor(Asset& selectedAsset, FrameInfo& frameInfo);

        void addComponent(FrameInfo& frameInfo);
    };
}
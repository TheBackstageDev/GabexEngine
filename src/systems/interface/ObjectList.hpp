#pragma once

#include <imgui/imgui.h>
#include "GWFrameInfo.hpp"
#include "GWMaterialHandler.hpp"

#include <vector>
#include <string>

namespace GWIN
{
    class GWObjectList
    {
    public:
        GWObjectList(std::unique_ptr<GWMaterialHandler> &materialHandler) : materialHandler(materialHandler) {};
        void Draw(FrameInfo& frameInfo);

        uint32_t getSelectedObject() { return selectedItem; }
    private:
        std::unique_ptr<GWMaterialHandler> &materialHandler;

        int selectedItem{-1}; //-1: none selected;
        int selectedMaterial = {0};
        bool transformOpen{true};
        bool isEditingName{false};
        bool rotationChanged{false};

        //Value Buffers
        char nameBuffer[1000]{""};
        glm::vec3 positionBuffer{0.0f, 0.0f, 0.0f};
        glm::vec3 rotationBuffer{0.0f, 0.0f, 0.0f};
        float scaleBuffer = 1.f;

        void inputRotation(GWGameObject &selectedObject);
        void inputPosition(GWGameObject &selectedObject);
        void inspectorGuis(GWGameObject &selectedObject);
        void addComponent(FrameInfo& frameInfo);
    };
}
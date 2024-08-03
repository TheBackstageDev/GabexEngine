#pragma once

#include <imgui/imgui.h>
#include "GWFrameInfo.hpp"

#include <vector>
#include <string>

namespace GWIN
{
    class GWObjectList
    {
    public:
        void Draw(FrameInfo& frameInfo);

        uint32_t getSelectedObject() { return selectedItem; }
    private:
        int selectedItem{-1}; //-1: none selected;
        bool transformOpen{true};
        bool isEditingName{false};

        //Value Buffers
        char nameBuffer[1000]{""};
        glm::vec3 positionBuffer{0.0f, 0.0f, 0.0f};
        glm::vec3 rotationBuffer{0.0f, 0.0f, 0.0f};
        float scaleBuffer = 1.f;

        void inputRotation(GWGameObject &selectedObject);
        void inputPosition(GWGameObject &selectedObject);
        void transformGui(GWGameObject &selectedObject);
    };
}
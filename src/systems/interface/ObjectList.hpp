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
    private:
        int selectedItem{-1}; //-1: none selected;
        bool transformOpen{true};

        void inputRotation(glm::vec3 &rotationBuffer, GWGameObject &selectedObject);
        void inputPosition(glm::vec3 &positionBuffer, GWGameObject &selectedObject);
    };
}
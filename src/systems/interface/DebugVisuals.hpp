#pragma once

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include <GWFrameInfo.hpp>

#include <unordered_map>
#include <array>

namespace GWIN
{
    struct DebugVertex
    {
        ImVec2 position{0.0f, 0.0f};
        ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    };

    struct DebugElementInfo
    {
        glm::vec3 wPosition; // World Position
        glm::quat rotation; // Rotation Quaternion
        glm::vec3 scale;
    };

    // the Object that will be rendered on the Imgui Screen
    class DebugElement 
    {
    public:
        DebugElement(std::vector<DebugVertex>& vertices, uint32_t& id);

        void drawElement(DebugElementInfo& newInfo);
    private:
        TransformComponent transform{};

        void calcVerticesPos(); //Calculates to Screen-space

        std::vector<DebugVertex>& vertices;
        uint32_t& id;
    };

    class DebugVisuals
    {
    public:
        DebugVisuals() {}

        void draw(FrameInfo& frameInfo);

        static void setDrawList(ImDrawList *newDrawList) { drawList = newDrawList; };
        static ImDrawList *getDrawList() { return drawList; };

    private:
        //Shapes creation
        void createSphere(GWGameObject &gameObject);
        void createCone(GWGameObject &gameObject);
        void createParalelepiped(GWGameObject &gameObject);

        std::unordered_map<uint32_t, DebugElement> debugMeshes;

        static ImDrawList* drawList;
    };
}
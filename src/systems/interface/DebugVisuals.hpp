#pragma once

#include <imgui/imgui.h>
#include <imguizmo/ImGuizmo.h>
#include <GWFrameInfo.hpp>

#include <unordered_map>
#include <algorithm>
#include <array>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GWIN
{
    struct DebugVertex
    {
        ImVec2 position{0.0f, 0.0f};
        ImVec4 color{1.0f, 1.0f, 1.0f, 1.0f};
    };

    // the Object that will be rendered on the Imgui Screen
    class DebugElement 
    {
    public:
        DebugElement(std::vector<glm::vec3> verticesPositions, const std::vector<uint32_t> indices, uint32_t id);

        void drawElement(FrameInfo &frameInfo);

    private:
        void calcVerticesPos(FrameInfo& frameInfo); // Calculates to Screen-space
        void drawLines();

        std::vector<DebugVertex> vertices;
        const std::vector<glm::vec3> verticesPositions;
        const std::vector<uint32_t> indices;
        uint32_t id;
    };

    class DebugVisuals
    {
    public:
        DebugVisuals();

        void draw(FrameInfo& frameInfo);

        static void setRect(float x, float y, float width, float height) { rectBounds = ImVec4(x, y, width, height); };
        static void setDrawList(ImDrawList *newDrawList) { drawList = newDrawList; };
        static ImDrawList *getDrawList() { return drawList; };
        static ImVec4 getRect() { return rectBounds; }

    private:
        friend class DebugElement;
        //Shapes creation
        void createFrustum(uint32_t id, const GWIN::GWCamera &camera);
        void createSphere(uint32_t id);
        void createCone(uint32_t id);
        void createParalelepiped(uint32_t id);

        std::unordered_map<uint32_t, DebugElement> debugMeshes;

        std::unordered_map<uint32_t, uint32_t> processedIds;

        static ImDrawList* drawList;
        static ImVec4 rectBounds;
    };
}
#define NOMINMAX
#include "DebugVisuals.hpp"

#include <iostream>
#include <limits>

namespace GWIN
{
    ImDrawList* DebugVisuals::drawList = nullptr;
    ImVec4 DebugVisuals::rectBounds = ImVec4(0.f, 0.f, 0.f, 0.f);

    ////// Debug Element //////

    DebugElement::DebugElement(std::vector<glm::vec3> verticesPositions, const std::vector<uint32_t> indices, uint32_t id) 
    : verticesPositions(verticesPositions), vertices(vertices), indices(indices), id(id)
    {
        vertices.resize(verticesPositions.size());
    }

    void DebugElement::drawElement(FrameInfo &frameInfo)
    {
        calcVerticesPos(frameInfo);
        drawLines();
    }

    void DebugElement::calcVerticesPos(FrameInfo &frameInfo)
    {
        GWGameObject &obj = frameInfo.currentInfo.gameObjects.at(id);
        glm::mat4 transformMatrix = obj.transform.mat4();
        glm::mat4 viewMatrix = frameInfo.currentInfo.currentCamera.getView();
        glm::mat4 projectionMatrix = frameInfo.currentInfo.currentCamera.getProjection();

        glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * transformMatrix;

        ImVec4 rectBounds = DebugVisuals::getRect();

        for (size_t i = 0; i < verticesPositions.size(); ++i)
        {
            glm::vec4 clipPos = mvpMatrix * glm::vec4(verticesPositions[i], 1.0f);
            glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;

            if (ndcPos.z < -1.0f || ndcPos.z > 1.0f)
                continue;

            glm::vec2 screenPos = ((glm::vec2(ndcPos) + 1.0f) * 0.5f) * glm::vec2(rectBounds.z, rectBounds.w);

            screenPos.x += rectBounds.x;
            screenPos.y += rectBounds.y;

            if (screenPos.x >= rectBounds.x && screenPos.x <= rectBounds.x + rectBounds.z &&
                screenPos.y >= rectBounds.y && screenPos.y <= rectBounds.y + rectBounds.w)
            {
                vertices[i].position = ImVec2(screenPos.x, screenPos.y);
            } else {
                vertices[i].position = ImVec2(0, 0);
            }
        }
    }

    void DebugElement::drawLines()
    {
        ImDrawList* drawList = DebugVisuals::getDrawList();

        ImU32 color = ImColor(1.0f, 1.0f, 1.0f, 1.0f);

        for (size_t i = 0; i < indices.size(); i += 2)
        {
            const DebugVertex &v0 = vertices[indices[i]];
            const DebugVertex &v1 = vertices[indices[i + 1]];

            if (v0.position.x == 0.f || v0.position.y == 0.f || v1.position.x == 0.f || v1.position.y == 0.f)
                continue;

            drawList->AddLine(v0.position, v1.position, color, .5f);
        }
    }

    ////// Debug Visuals //////

    DebugVisuals::DebugVisuals()
    {
    }

    void DebugVisuals::draw(FrameInfo& frameInfo) 
    {
        if (drawList == nullptr)
            return;

        auto& currentInfo = frameInfo.currentInfo;

        for (auto it = currentInfo.gameObjects.begin(); it != currentInfo.gameObjects.end(); it++)
        {
            const auto &[id, obj] = *it;

            auto findIt = processedIds.find(id);

            if (findIt == processedIds.end())
            {
                if (obj.light)
                {
                    if (obj.light->cutOffAngle == 0.0f)
                    {
                        createSphere(id);
                    }
                    else
                    {
                        createCone(id);
                    }

                    processedIds.emplace(id, id);
                }
            }
        }

        for (auto it = debugMeshes.begin(); it != debugMeshes.end();)
        {
            auto &[id, element] = *it;

            if (currentInfo.gameObjects.find(id) == currentInfo.gameObjects.end())
            {
                it = debugMeshes.erase(it); // `erase` returns the iterator to the next element
            }
            else
            {
                if (currentInfo.currentCamera.isPointInFrustum(currentInfo.gameObjects.at(id).transform.translation))
                {
                    element.drawElement(frameInfo);
                }

                ++it;
            }
        }
    }

    void DebugVisuals::createFrustum(uint32_t id, const GWIN::GWCamera &camera) // TO DO
    {
    }

    void DebugVisuals::createCone(uint32_t id)
    {
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        const int segments = 32;                  // Number of segments for the base circle
        const float radius = 0.5f;                // Radius of the base
        const float height = 1.0f;                // Height of the cone
        const glm::vec3 apex(0.0f, 0.0f, 0.0f); // Apex of the cone (tip)

        // Add the apex point (tip of the cone)
        vertices.push_back(apex);

        // Base circle
        for (int i = 0; i < segments; ++i)
        {
            float theta = glm::two_pi<float>() * float(i) / float(segments);

            float x = radius * cosf(theta);
            float z = radius * sinf(theta);
            float y = -height; // Base is at y = 0

            vertices.push_back(glm::vec3(x, y, z));

            // Circle indices (connect vertices to form the base)
            indices.push_back(i + 1);
            indices.push_back((i + 1) % segments + 1);

            indices.push_back(0);
            indices.push_back(i + 1); 
        }

        DebugElement element{vertices, indices, id};
        debugMeshes.emplace(id, std::move(element));
    }

    void DebugVisuals::createSphere(uint32_t id)
    {
        std::vector<glm::vec3> vertices;
        std::vector<uint32_t> indices;

        const int stacks = 10; 
        const int slices = 20; 
        const float radius = 0.5f;

        for (int i = 0; i <= stacks; ++i)
        {
            float phi = glm::pi<float>() * float(i) / float(stacks); 

            for (int j = 0; j <= slices; ++j)
            {
                float theta = glm::two_pi<float>() * float(j) / float(slices); 

                float x = radius * sinf(phi) * cosf(theta);
                float y = radius * cosf(phi);
                float z = radius * sinf(phi) * sinf(theta);

                // Add vertex as glm::vec3
                vertices.push_back(glm::vec3(x, y, z));

                // Connect longitude lines (vertical grid lines along XZ-plane)
                if (i < stacks)
                {
                    int first = i * (slices + 1) + j;
                    int second = first + slices + 1;
                    indices.push_back(first);
                    indices.push_back(second);
                }

                // Connect latitude lines (horizontal grid lines around Y-axis)
                if (j < slices)
                {
                    int first = i * (slices + 1) + j;
                    int second = first + 1;
                    indices.push_back(first);
                    indices.push_back(second);
                }
            }
        }

        DebugElement element{vertices, indices, id};
        debugMeshes.emplace(id, std::move(element));
    }

    void DebugVisuals::createParalelepiped(uint32_t id) 
    {
        std::vector<glm::vec3> vertices = {
            // Back face
            {-0.25f, -0.25f, -0.25f}, // 0: bottom-left-back
            {0.25f, -0.25f, -0.25f},  // 1: bottom-right-back
            {0.25f, 0.25f, -0.25f},   // 2: top-right-back
            {-0.25f, 0.25f, -0.25f},  // 3: top-left-back
            // Front face
            {-0.25f, -0.25f, 0.25f}, // 4: bottom-left-front
            {0.25f, -0.25f, 0.25f},  // 5: bottom-right-front
            {0.25f, 0.25f, 0.25f},   // 6: top-right-front
            {-0.25f, 0.25f, 0.25f}   // 7: top-left-front
        };

        std::vector<uint32_t> indices = {
            // Back face
            0, 1, 1, 2, 2, 3, 3, 0,
            // Front face
            4, 5, 5, 6, 6, 7, 7, 4,
            // Connecting lines between front and back faces
            0, 4, 1, 5, 2, 6, 3, 7};
        DebugElement element{vertices, indices, id};
        debugMeshes.emplace(id, std::move(element));
    }
}
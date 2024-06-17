#include "GWApp.hpp"
#include <iostream>

#include <glm/gtc/constants.hpp>

namespace GWIN
{
    GWapp::GWapp()
    {
        loadGameObjects();
    }

    void GWapp::run()
    {
        RenderSystem renderSystem{GDevice, GRenderer.getRenderPass()};
        while (!GWindow.shouldClose())
        {
            glfwPollEvents();

            if(auto commandBuffer = GRenderer.startFrame())
            {
                GRenderer.startSwapChainRenderPass(commandBuffer);
                renderSystem.renderGameObjects(commandBuffer, gameObjects);
                GRenderer.endSwapChainRenderPass(commandBuffer);
                GRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(GDevice.device());
    }

    void sierpinski(const std::vector<GWModel::Vertex> &triangle, std::vector<GWModel::Vertex> &vertices, int depth)
    {
        if (depth == 0)
        {
            vertices.insert(vertices.end(), triangle.begin(), triangle.end());
            return;
        }

        GWModel::Vertex mid1 = {{
            (triangle[0].position.x + triangle[1].position.x) / 2,
            (triangle[0].position.y + triangle[1].position.y) / 2
        }, triangle[1].color};

        GWModel::Vertex mid2 = {{
            (triangle[1].position.x + triangle[2].position.x) / 2,
            (triangle[1].position.y + triangle[2].position.y) / 2
        }, triangle[1].color};

        GWModel::Vertex mid3 = {{
            (triangle[2].position.x + triangle[0].position.x) / 2,
            (triangle[2].position.y + triangle[0].position.y) / 2
        },  triangle[1].color};

        //Triangles
        std::vector<GWModel::Vertex> tri1 = {triangle[0], mid1, mid3};
        std::vector<GWModel::Vertex> tri2 = {mid1, triangle[1], mid2};
        std::vector<GWModel::Vertex> tri3 = {mid2, mid3, triangle[2]};

        sierpinski(tri1, vertices, depth - 1);
        sierpinski(tri2, vertices, depth - 1);
        sierpinski(tri3, vertices, depth - 1);
    }

    void GWapp::loadGameObjects()
        {
            std::vector<GWModel::Vertex> vertices;
            std::vector<GWModel::Vertex> BaseTriangle{
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

            sierpinski(BaseTriangle, vertices, 5);
            auto Model = std::make_shared<GWModel>(GDevice, vertices);
            
            auto triangle = GWGameObject::createGameObject();
            triangle.color = {1.f, .5f, 1.f};
            triangle.model = Model;
            triangle.transform2d.translation.x = .25f;
            triangle.transform2d.scale = {1.f, 1.f}; 
            triangle.transform2d.rotation = .25f * glm::two_pi<float>();

            gameObjects.push_back(std::move(triangle));
        }
    }
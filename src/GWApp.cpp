#include "GWApp.hpp"
#include <stdexcept>
#include <array>
#include <iostream>
#include <functional>
#include <cassert>

#include <glm/gtc/constants.hpp>

namespace GWIN
{
    struct SpushConstant
    {
        glm::mat2 transform{1.f};
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };

    GWapp::GWapp()
    {
        loadGameObjects();
        createPipelineLayout();
        recreateSwapChain();
        createCommandBuffers();
    }

    GWapp::~GWapp()
    {
        vkDestroyPipelineLayout(GDevice.device(), pipelineLayout, nullptr);
    }

    void GWapp::run()
    {
        std::cout << "Max Push Constant Size: " << GDevice.properties.limits.maxPushConstantsSize << "\n";
        while (!GWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();
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
            std::vector<GWModel::Vertex> vertices{
                {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
            auto Model = std::make_shared<GWModel>(GDevice, vertices);

            // https://www.color-hex.com/color-palette/5361
            std::vector<glm::vec3> colors{
                {1.f, .7f, .73f},
                {1.f, .87f, .73f},
                {1.f, 1.f, .73f},
                {.73f, 1.f, .8f},
                {.73, .88f, 1.f} //
            };
            for (auto &color : colors)
            {
                color = glm::pow(color, glm::vec3{2.2f});
            }
            for (int i = 0; i < 100; i++)
            {
                auto triangle = GWGameObject::createGameObject();
                triangle.model = Model;
                triangle.transform2d.scale = glm::vec2(.5f) + i * 0.010f;
                triangle.transform2d.rotation = i * glm::pi<float>() * .025f;
                triangle.color = colors[i % colors.size()];
                gameObjects.push_back(std::move(triangle));
            }
        }

        void GWapp::createPipelineLayout()
        {
            VkPushConstantRange pushConstant{};
            pushConstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            pushConstant.offset = 0;
            pushConstant.size = sizeof(SpushConstant);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = 0;
            pipelineLayoutInfo.pSetLayouts = nullptr;
            pipelineLayoutInfo.pushConstantRangeCount = 1;
            pipelineLayoutInfo.pPushConstantRanges = &pushConstant;

            if (vkCreatePipelineLayout(GDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to Create Pipeline Layout!");
            }
        }

        void GWapp::createPipeline()
        {
            assert(swapChain != nullptr && "Cannot create pipeline before swap chain");
            assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

            PipelineConfigInfo pipelineConfig{};
            GPipeLine::defaultPipelineConfigInfo(pipelineConfig);
            pipelineConfig.renderPass = swapChain->getRenderPass();
            pipelineConfig.pipelineLayout = pipelineLayout;

            Pipeline = std::make_unique<GPipeLine>(
                GDevice,
                "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.vert.spv",
                "C:/Users/cleve/OneDrive/Documents/GitHub/GabexEngine/src/shaders/shader.frag.spv",
                pipelineConfig);
        }

        void GWapp::createCommandBuffers()
        {
            commandBuffers.resize(swapChain->imageCount());

            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandPool = GDevice.getCommandPool();
            allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

            if (vkAllocateCommandBuffers(GDevice.device(), &allocInfo, commandBuffers.data()) !=
                VK_SUCCESS)
            {
                throw std::runtime_error("failed to allocate command buffers!");
            }

        }

        void GWapp::recreateSwapChain()
        {
            auto extent = GWindow.getExtent();

            while(extent.height == 0 || extent.width == 0)
            {
                extent = GWindow.getExtent();
                glfwWaitEvents();
            }

            vkDeviceWaitIdle(GDevice.device());
            swapChain = std::make_unique<GWinSwapChain>(GDevice, extent);
            createPipeline();
        }

        void GWapp::recordCommandBuffer(int imageIndex)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffer!");
            }

            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = swapChain->getRenderPass();
            renderPassInfo.framebuffer = swapChain->getFrameBuffer(imageIndex);

            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
            viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
            vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
            vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

            renderGameObjects(commandBuffers[imageIndex]);

            vkCmdEndRenderPass(commandBuffers[imageIndex]);
            if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to record command buffer!");
            }
        }

        void GWapp::renderGameObjects(VkCommandBuffer commandBuffer)
        {
            // update
            int i = 0;
            for (auto &obj : gameObjects)
            {
                i += 1;
                obj.transform2d.rotation =
                    glm::mod<float>(obj.transform2d.rotation + 0.001f * i, 2.f * glm::pi<float>());
            }

            // render
            Pipeline->bind(commandBuffer);
            for (auto &obj : gameObjects)
            {
                SpushConstant push{};
                push.offset = obj.transform2d.translation;
                push.color = obj.color;
                push.transform = obj.transform2d.mat2();

                vkCmdPushConstants(
                    commandBuffer,
                    pipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    sizeof(SpushConstant),
                    &push);
                obj.model->bind(commandBuffer);
                obj.model->draw(commandBuffer);
            }
        }

        void GWapp::drawFrame()
        {
            uint32_t imageIndex;
            auto result = swapChain->acquireNextImage(&imageIndex);

            if (result == VK_ERROR_OUT_OF_DATE_KHR)
            {
                recreateSwapChain();
                return;
            }

            if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
            {
                throw std::runtime_error("failed to acquire swap chain image!");
            }


            recordCommandBuffer(imageIndex);
            result = swapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);

            if (result == VK_SUBOPTIMAL_KHR || result == VK_ERROR_OUT_OF_DATE_KHR || GWindow.hasWindowBeenResized())
            {
                GWindow.frameBufferResizedFlagReset();
                recreateSwapChain();
                return;
            }

            if (result != VK_SUCCESS)
            {
                throw std::runtime_error("failed to present swap chain image!");
            }
        }
    }
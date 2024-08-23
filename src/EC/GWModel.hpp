#pragma once

#include "../GWDevice.hpp"
#include "../GWBuffer.hpp"

#include <vector>
#include <memory>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace GWIN
{
    class GWModel
    {
    public:
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec3 normal;
            glm::vec2 uv;
            glm::vec3 tangent;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        struct Builder
        {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};
        };

        GWModel(GWinDevice &device, const GWModel::Builder &builder);
        ~GWModel();

        GWModel(const GWModel &) = delete;
        GWModel &operator=(const GWModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

        std::string getPath() { return pathToModel; }
        void setPath(const std::string path) { pathToModel = path; }
    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        void createIndexBuffers(const std::vector<uint32_t> &indices);

        GWinDevice &device;

        std::string pathToModel = "";

        std::unique_ptr<GWBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<GWBuffer> indexBuffer;
        uint32_t indexCount;

        bool hasIndexBuffers{false};
    };
}
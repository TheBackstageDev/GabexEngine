#include "GWModel.hpp"

// std

#include <cassert>
#include <cstring>

namespace GWIN
{
    GWModel::GWModel(GWinDevice &device, const std::vector<Vertex> &vertices) : device(device)
    {
        createVertexBuffers(vertices);
    }

    GWModel::~GWModel()
    {
        vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
        vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
    }

    void GWModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Number of vertices has to be atleast 3!");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        device.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertexBuffer, vertexBufferMemory);

        void *data;
        vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        std::memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(device.device(), vertexBufferMemory);
    }

    void GWModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
        vkCmdBindVertexBuffers(commandBuffer, 1, 1, buffers, offsets);
    }

    void GWModel::draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    std::vector<VkVertexInputBindingDescription> GWModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> BindingDescriptions(2);
        BindingDescriptions[0].binding = 0;
        BindingDescriptions[0].stride = sizeof(Vertex);
        BindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        BindingDescriptions[1].binding = 1;
        BindingDescriptions[1].stride = sizeof(Vertex);
        BindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return BindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GWModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> AttributeDescriptions(2);
        AttributeDescriptions[0].binding = 0;
        AttributeDescriptions[0].location = 0;
        AttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        AttributeDescriptions[0].offset = offsetof(Vertex, position);
        AttributeDescriptions[1].binding = 1;
        AttributeDescriptions[1].location = 1;
        AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[1].offset = offsetof(Vertex, color);
        return AttributeDescriptions;
    }
}
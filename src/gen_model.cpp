#include "gen_model.hpp"

// std
#include <cassert>

namespace gen
{
    GenModel::GenModel(GenDevice &device, const std::vector<Vertex> &vertices)
        : genDevice{device}
    {
        createVertexBuffers(vertices);
    }
    GenModel::~GenModel()
    {
        vkDestroyBuffer(genDevice.device(), vertexBuffer, nullptr);
        vkFreeMemory(genDevice.device(), vertexBufferMemory, nullptr);
    }

    void GenModel::createVertexBuffers(const std::vector<Vertex> &vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "vertexcount must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

        genDevice.createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, // host is CPU
            vertexBuffer,
            vertexBufferMemory);

        void *data;
        vkMapMemory(genDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
        vkUnmapMemory(genDevice.device(), vertexBufferMemory);
    }

    void GenModel::draw(VkCommandBuffer commandBuffer)
    {
        vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
    }

    void GenModel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = {vertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets); // records to cammand buffer to bind 1 vertex buffer starting at binding 0 with an offset of 0 into the buffer
    }

    std::vector<VkVertexInputBindingDescription> GenModel::Vertex::getBindingDescriptions()
    {
        std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(Vertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    std::vector<VkVertexInputAttributeDescription> GenModel::Vertex::getAttributeDescriptions()
    {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1; // has to match the location in vertex shader
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        return attributeDescriptions;
    }
}
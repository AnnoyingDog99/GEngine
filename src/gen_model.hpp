#pragma once

#include "gen_device.hpp"

// glm
#define GLM_FORCE_RADIANS           // glm functions will except values in radians, not degrees
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm functions will expect depth values to be in range [0, 1]
#include <glm/glm.hpp>

// std
#include <vector>

namespace gen
{
    class GenModel
    {
    public:
        struct Vertex
        {
            glm::vec2 position;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };
        GenModel(GenDevice &device, const std::vector<Vertex> &vertices);
        ~GenModel();

        GenModel(const GenModel &) = delete;
        GenModel &operator=(const GenModel &) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex> &vertices);
        GenDevice &genDevice;
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        uint32_t vertexCount;
    };
} // namespace gen

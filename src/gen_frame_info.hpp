#pragma once

#include "gen_camera.hpp"
#include "gen_game_object.hpp"

// vulkan
#include <vulkan/vulkan.h>

namespace gen
{

#define MAX_LIGHTS 10

    struct PointLight
    {
        glm::vec4 position{}; // ignore w
        glm::vec4 color{};    // w is intensity
    };
    struct GlobalUbo
    {
        glm::mat4 projection{1.f};
        glm::mat4 view{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
        PointLight pointLights[MAX_LIGHTS];
        int numLights;
    };

    struct FrameInfo
    {
        int frameIndex;
        float frameTime;
        VkCommandBuffer commandBuffer;
        GenCamera &camera;
        VkDescriptorSet globalDescriptorSet;
        GenGameObject::Map &gameObjects;
    };
}
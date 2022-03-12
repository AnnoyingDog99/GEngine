#pragma once

#include "gen_camera.hpp"
#include "gen_game_object.hpp"

// vulkan
#include <vulkan/vulkan.h>

namespace gen
{
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
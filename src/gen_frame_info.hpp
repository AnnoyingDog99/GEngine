#pragma once

#include "gen_camera.hpp"

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
    };
}
#pragma once

#include "gen_camera.hpp"
#include "gen_device.hpp"
#include "gen_game_object.hpp"
#include "gen_pipeline.hpp"

// std
#include <memory>
#include <vector>

namespace gen
{
    class SimpleRenderSystem
    {

    public:
        SimpleRenderSystem(GenDevice &device, VkRenderPass renderPass);
        ~SimpleRenderSystem();

        SimpleRenderSystem(const SimpleRenderSystem &) = delete;
        SimpleRenderSystem &operator=(const SimpleRenderSystem &) = delete;

        void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<GenGameObject> &gameObjects, const GenCamera &camera);

    private:
        void createPipelineLayout();
        void createPipeline(VkRenderPass renderPass);

        GenDevice &genDevice;

        std::unique_ptr<GenPipeline> genPipeline;
        VkPipelineLayout pipelineLayout;
    };
}
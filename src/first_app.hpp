#pragma once

#include "gen_window.hpp"
#include "gen_pipeline.hpp"
#include "gen_device.hpp"
#include "gen_swap_chain.hpp"
#include "gen_model.hpp"

// std
#include <memory>
#include <vector>

namespace gen
{
    class FirstApp
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        FirstApp();
        ~FirstApp();

        FirstApp(const FirstApp &) = delete;
        FirstApp &operator=(const FirstApp &) = delete;

        void run();

    private:
        void loadModels();
        void createPipelineLayout();
        void createPipeline();
        void createCommandBuffers();
        void freeCommandBuffers();
        void drawFrame();
        void recreateSwapChain();
        void recordCommandBuffer(int imageIndex);

        GenWindow genWindow{WIDTH, HEIGHT, "Vulkan window"};
        GenDevice genDevice{genWindow};
        std::unique_ptr<GenSwapChain> genSwapChain;

        std::unique_ptr<GenPipeline> genPipeline;
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<GenModel> genModel;
    };
}
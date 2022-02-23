#pragma once

#include "gen_window.hpp"
#include "gen_pipeline.hpp"
#include "gen_device.hpp"

namespace gen
{
    class FirstApp
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        void run();

    private:
        GenWindow genWindow{WIDTH, HEIGHT, "Vulkan window"};
        GenDevice genDevice{genWindow};
        GenPipeline genPipeline{
            genDevice,
            "shaders/simple_shader.vert.spv",
            "shaders/simple_shader.frag.spv",
            GenPipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
    };
}
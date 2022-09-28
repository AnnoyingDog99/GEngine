#pragma once

#include "gen_device.hpp"
#include "gen_game_object.hpp"
#include "gen_window.hpp"
#include "gen_renderer.hpp"
#include "gen_descriptors.hpp"

// std
#include <memory>
#include <vector>

namespace gen
{
    class App
    {

    public:
        static constexpr int WIDTH = 800;
        static constexpr int HEIGHT = 600;

        App();
        ~App();

        App(const App &) = delete;
        App &operator=(const App &) = delete;

        void run();

    private:
        void loadGameObjects();

        GenWindow genWindow{WIDTH, HEIGHT, "Vulkan window"};
        GenDevice genDevice{genWindow};
        GenRenderer genRenderer{genWindow, genDevice};

        // order matters (pool should be destroyed before the devices)
        std::unique_ptr<GenDescriptorPool> globalPool{};
        GenGameObject::Map gameObjects;
    };
}
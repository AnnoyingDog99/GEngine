#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "gen_camera.hpp"
#include "simple_render_system.hpp"
#include "gen_buffer.hpp"

// glm
#define GLM_FORCE_RADIANS           // glm functions will except values in radians, not degrees
#define GLM_FORCE_DEPTH_ZERO_TO_ONE // glm functions will expect depth values to be in range [0, 1]
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <stdexcept>
#include <chrono>
#include <array>
#include <cassert>

namespace gen
{

    struct GlobalUbo
    {
        glm::mat4 projectionView{1.f};
        glm::vec4 ambientLightColor{1.f, 1.f, 1.f, .02f};
        glm::vec3 lightPosition{-1.f};
        alignas(16) glm::vec4 lightColor{1.f}; // w is light intensity
    };

    FirstApp::FirstApp()
    {
        globalPool =
            GenDescriptorPool::Builder(genDevice)
                .setMaxSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT)
                .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GenSwapChain::MAX_FRAMES_IN_FLIGHT)
                .build();
        loadGameObjects();
    }

    FirstApp::~FirstApp() {}

    void FirstApp::run()
    {

        std::vector<std::unique_ptr<GenBuffer>> uboBuffers(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < uboBuffers.size(); i++)
        {
            uboBuffers[i] = std::make_unique<GenBuffer>(
                genDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
            uboBuffers[i]->map();
        }

        auto globalSetLayout =
            GenDescriptorSetLayout::Builder(genDevice)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .build();

        std::vector<VkDescriptorSet> globalDescriptorSets(GenSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++)
        {
            auto bufferInfo = uboBuffers[i]->descriptorInfo();
            GenDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .build(globalDescriptorSets[i]);
        }

        SimpleRenderSystem simpleRenderSystem{
            genDevice,
            genRenderer.getSwapChainRenderPass(),
            globalSetLayout->getDescriptorSetLayout()};
        GenCamera camera{};

        auto viewerObject = GenGameObject::createGameObject();
        viewerObject.transform.translation.z = -2.5f;
        KeyboardMovementController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

        while (!genWindow.shouldClose())
        {
            glfwPollEvents();

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(genWindow.getGLFWWindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = genRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

            if (auto commandBuffer = genRenderer.beginFrame())
            {
                int frameIndex = genRenderer.getFrameIndex();
                FrameInfo frameInfo{
                    frameIndex,
                    frameTime,
                    commandBuffer,
                    camera,
                    globalDescriptorSets[frameIndex]};

                // update
                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                genRenderer.beginSwapChainRenderPass(commandBuffer);
                simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
                genRenderer.endSwapChainRenderPass(commandBuffer);
                genRenderer.endFrame();
            }
        }

        vkDeviceWaitIdle(genDevice.device()); // cpu will block till all gpu operations are finished
    }

    void FirstApp::loadGameObjects()
    {
        std::shared_ptr<GenModel> genModel = GenModel::createModelFromFile(genDevice, "models/flat_vase.obj");

        auto flatVase = GenGameObject::createGameObject();
        flatVase.model = genModel;
        flatVase.transform.translation = {-.5f, .5f, 0.f};
        flatVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.push_back(std::move(flatVase));

        genModel = GenModel::createModelFromFile(genDevice, "models/smooth_vase.obj");
        auto smoothVase = GenGameObject::createGameObject();
        smoothVase.model = genModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.push_back(std::move(smoothVase));

        genModel = GenModel::createModelFromFile(genDevice, "models/quad.obj");
        auto floor = GenGameObject::createGameObject();
        floor.model = genModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.push_back(std::move(floor));
    }
}
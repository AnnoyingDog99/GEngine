#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "gen_camera.hpp"
#include "gen_buffer.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/point_light_system.hpp"

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
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
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

        PointLightSystem pointLightSystem{
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
                    globalDescriptorSets[frameIndex],
                    gameObjects};

                // update
                GlobalUbo ubo{};
                ubo.projection = camera.getProjection();
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                pointLightSystem.update(frameInfo, ubo);
                uboBuffers[frameIndex]->writeToBuffer(&ubo);
                uboBuffers[frameIndex]->flush();

                // render
                genRenderer.beginSwapChainRenderPass(commandBuffer);

                //order here matters, first solid, then semi-transparent
                simpleRenderSystem.renderGameObjects(frameInfo);
                pointLightSystem.render(frameInfo);

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
        gameObjects.emplace(flatVase.getId(), std::move(flatVase));

        genModel = GenModel::createModelFromFile(genDevice, "models/smooth_vase.obj");
        auto smoothVase = GenGameObject::createGameObject();
        smoothVase.model = genModel;
        smoothVase.transform.translation = {.5f, .5f, 0.f};
        smoothVase.transform.scale = {3.f, 1.5f, 3.f};
        gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

        genModel = GenModel::createModelFromFile(genDevice, "models/quad.obj");
        auto floor = GenGameObject::createGameObject();
        floor.model = genModel;
        floor.transform.translation = {0.f, .5f, 0.f};
        floor.transform.scale = {3.f, 1.f, 3.f};
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::vector<glm::vec3> lightColors{
            {1.f, .1f, .1f},
            {.1f, .1f, 1.f},
            {.1f, 1.f, .1f},
            {1.f, 1.f, .1f},
            {.1f, 1.f, 1.f},
            {1.f, 1.f, 1.f} //
        };

        for (int i = 0; i < lightColors.size(); i++)
        {
            auto pointLight = GenGameObject::makePointLight(0.2f);
            pointLight.color = lightColors[i];
            auto rotateLight = glm::rotate(glm::mat4(1.f), (i * glm::two_pi<float>()) / lightColors.size(), {0.1, -1.f, 0.f});
            pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
            gameObjects.emplace(pointLight.getId(), std::move(pointLight));
        }
    }
}
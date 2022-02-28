#include "first_app.hpp"

// std
#include <stdexcept>
#include <array>

namespace gen
{

    FirstApp::FirstApp()
    {
        loadModels();
        createPipelineLayout();
        createPipeline();
        createCommandBuffers();
    }

    FirstApp::~FirstApp()
    {
        vkDestroyPipelineLayout(genDevice.device(), pipelineLayout, nullptr);
    }

    void FirstApp::run()
    {
        while (!genWindow.shouldClose())
        {
            glfwPollEvents();
            drawFrame();

            vkDeviceWaitIdle(genDevice.device()); // cpu will block till all gpu operations are finished
        }
    }

    void FirstApp::loadModels()
    {
        std::vector<GenModel::Vertex> vertices{
            {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
            {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};
        genModel = std::make_unique<GenModel>(genDevice, vertices);
    }

    void FirstApp::createPipelineLayout()
    {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr; // PipelineSetLayouts is used to pass data other than vertex data to our vertex and fragment shaders (textures, uniform buffers)
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // PushConstants are a way to very efficiently send a small amount of data to our shader programs
        if (vkCreatePipelineLayout(genDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create pipeline layout!");
        }
    }

    void FirstApp::createPipeline()
    {
        auto pipelineConfig = GenPipeline::defaultPipelineConfigInfo(genSwapChain.width(), genSwapChain.height());
        pipelineConfig.renderPass = genSwapChain.getRenderPass();
        pipelineConfig.pipelineLayout = pipelineLayout;
        genPipeline = std::make_unique<GenPipeline>(genDevice, "shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", pipelineConfig);
    }

    void FirstApp::createCommandBuffers()
    {

        commandBuffers.resize(genSwapChain.imageCount());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = genDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(genDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }

        for (int i = 0; i < commandBuffers.size(); i++)
        {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to begin recording command buffers!");
            }

            VkRenderPassBeginInfo renderpassInfo{};
            renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderpassInfo.renderPass = genSwapChain.getRenderPass();
            renderpassInfo.framebuffer = genSwapChain.getFrameBuffer(i);

            renderpassInfo.renderArea.offset = {0, 0};
            renderpassInfo.renderArea.extent = genSwapChain.getSwapChainExtent();

            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderpassInfo.pClearValues = clearValues.data();

            // record to command buffer
            vkCmdBeginRenderPass(commandBuffers[i], &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

            genPipeline->bind(commandBuffers[i]);
            genModel->bind(commandBuffers[i]);
            genModel->draw(commandBuffers[i]);

            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
            {
                std::runtime_error("failed to record command buffer!");
            }
        }
    }

    void FirstApp::drawFrame()
    {
        uint32_t imageIndex;
        auto result = genSwapChain.acquireNextImage(&imageIndex); // fetches the index of the frame we should render to next
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        result = genSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex); // submit the provided command buffer to our device graphics queue, while handeling cpu and gpu synchronization
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit command buffer!");
        }
    }
}
#include "gen_renderer.hpp"

// std
#include <stdexcept>
#include <array>
#include <cassert>

namespace gen
{

    GenRenderer::GenRenderer(GenWindow &window, GenDevice &device) : genWindow{window}, genDevice{device}
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    GenRenderer::~GenRenderer()
    {
        freeCommandBuffers();
    }

    void GenRenderer::recreateSwapChain()
    {
        auto extent = genWindow.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = genWindow.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(genDevice.device());
        genSwapChain = nullptr;
        if (genSwapChain == nullptr)
        {
            genSwapChain = std::make_unique<GenSwapChain>(genDevice, extent);
        }
        else
        {
            std::shared_ptr<GenSwapChain> oldSwapChain = std::move(genSwapChain);
            genSwapChain = std::make_unique<GenSwapChain>(genDevice, extent, oldSwapChain);

            if (!oldSwapChain->compareSwapFormats(*genSwapChain.get()))
            {
                throw std::runtime_error("Swap chain image(or depth) format changed!");
                // instead of throwing an error, setup a callback notifying the app that a new incompatable renderpass has been created
            }
        }
    }

    void GenRenderer::createCommandBuffers()
    {

        commandBuffers.resize(GenSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = genDevice.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

        if (vkAllocateCommandBuffers(genDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void GenRenderer::freeCommandBuffers()
    {
        vkFreeCommandBuffers(genDevice.device(), genDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
        commandBuffers.clear();
    }

    VkCommandBuffer GenRenderer::beginFrame()
    {
        assert(!isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = genSwapChain->acquireNextImage(&currentImageIndex); // fetches the index of the frame we should render to next

        if (result == VK_ERROR_OUT_OF_DATE_KHR) // error can occur after the window had been resized
        {
            recreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        isFrameStarted = true;

        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffers!");
        }
        return commandBuffer;
    }
    void GenRenderer::endFrame()
    {
        assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
        {
            std::runtime_error("failed to record command buffer!");
        }

        auto result = genSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex); // submit the provided command buffer to our device graphics queue, while handeling cpu and gpu synchronization
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || genWindow.wasWindowResized())
        {
            genWindow.resetWindowResizedFlag();
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        isFrameStarted = false;
        currentFrameIndex = (currentFrameIndex + 1) % GenSwapChain::MAX_FRAMES_IN_FLIGHT;
    }
    void GenRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a diffrent frame");

        VkRenderPassBeginInfo renderpassInfo{};
        renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderpassInfo.renderPass = genSwapChain->getRenderPass();
        renderpassInfo.framebuffer = genSwapChain->getFrameBuffer(currentImageIndex);

        renderpassInfo.renderArea.offset = {0, 0};
        renderpassInfo.renderArea.extent = genSwapChain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderpassInfo.pClearValues = clearValues.data();

        // record to command buffer
        vkCmdBeginRenderPass(commandBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(genSwapChain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(genSwapChain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, genSwapChain->getSwapChainExtent()};
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    }
    void GenRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a diffrent frame");
        vkCmdEndRenderPass(commandBuffer);
    }
}
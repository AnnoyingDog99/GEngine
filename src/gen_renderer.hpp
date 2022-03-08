#pragma once

#include "gen_device.hpp"
#include "gen_swap_chain.hpp"
#include "gen_window.hpp"

// std
#include <memory>
#include <vector>
#include <cassert>

namespace gen
{
    class GenRenderer
    {

    public:
        GenRenderer(GenWindow &window, GenDevice &device);
        ~GenRenderer();

        GenRenderer(const GenRenderer &) = delete;
        GenRenderer &operator=(const GenRenderer &) = delete;

        VkRenderPass getSwapChainRenderPass() const
        {
            return genSwapChain->getRenderPass();
        }

        bool isFrameInProgress() const
        {
            return isFrameStarted;
        }

        VkCommandBuffer getCurrentCommandBuffer() const
        {
            assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
            return commandBuffers[currentFrameIndex];
        }

        int getFrameIndex() const
        {
            assert(isFrameStarted && "Cannot get frame index when frame not in progress");
            return currentFrameIndex;
        }

        VkCommandBuffer beginFrame();
        void endFrame();
        void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
        void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

    private:
        void createCommandBuffers();
        void freeCommandBuffers();
        void recreateSwapChain();

        GenWindow &genWindow;
        GenDevice &genDevice;
        std::unique_ptr<GenSwapChain> genSwapChain;
        std::vector<VkCommandBuffer> commandBuffers;

        uint32_t currentImageIndex;
        int currentFrameIndex;
        bool isFrameStarted;
    };
}
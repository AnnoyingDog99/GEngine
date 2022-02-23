#pragma once

#include "gen_device.hpp"

// std
#include <string>
#include <vector>

namespace gen
{

    // config is here so app can configure pipeline completely, also, config can now be easily shared between different pipelines
    struct PipelineConfigInfo
    {
        VkViewport viewport;
        VkRect2D scissor;
        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };

    class GenPipeline
    {
    public:
        GenPipeline(GenDevice &device, const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);
        ~GenPipeline();

        // delete copy constructors
        GenPipeline(const GenPipeline &) = delete;
        void operator=(const GenPipeline &) = delete;

        static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);

    private:
        static std::vector<char> readFile(const std::string &filepath);

        void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilepath, const PipelineConfigInfo &configInfo);

        void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule);

        GenDevice &genDevice;
        VkPipeline graphicsPipeline;
        VkShaderModule vertShaderModule;
        VkShaderModule fragShaderModule;
    };
}
#pragma once

#include "gen_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace gen
{
    class GenDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(GenDevice &genDevice) : genDevice{genDevice} {}

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<GenDescriptorSetLayout> build() const;

        private:
            GenDevice &genDevice;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        GenDescriptorSetLayout(
            GenDevice &genDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~GenDescriptorSetLayout();
        GenDescriptorSetLayout(const GenDescriptorSetLayout &) = delete;
        GenDescriptorSetLayout &operator=(const GenDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        GenDevice &genDevice;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class GenDescriptorWriter;
    };

    class GenDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(GenDevice &genDevice) : genDevice{genDevice} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<GenDescriptorPool> build() const;

        private:
            GenDevice &genDevice;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        GenDescriptorPool(
            GenDevice &genDevice,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~GenDescriptorPool();
        GenDescriptorPool(const GenDescriptorPool &) = delete;
        GenDescriptorPool &operator=(const GenDescriptorPool &) = delete;

        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
        GenDevice &genDevice;
        VkDescriptorPool descriptorPool;

        friend class GenDescriptorWriter;
    };

    class GenDescriptorWriter
    {
    public:
        GenDescriptorWriter(GenDescriptorSetLayout &setLayout, GenDescriptorPool &pool);

        GenDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        GenDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        GenDescriptorSetLayout &setLayout;
        GenDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} // namespace gen
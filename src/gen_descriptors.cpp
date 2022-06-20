#include "gen_descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace gen
{

    // *************** Descriptor Set Layout Builder *********************

    GenDescriptorSetLayout::Builder &GenDescriptorSetLayout::Builder::addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count)
    {
        assert(bindings.count(binding) == 0 && "Binding already in use");
        VkDescriptorSetLayoutBinding layoutBinding{
            .binding = binding,
            .descriptorType = descriptorType,
            .descriptorCount = count,
            .stageFlags = stageFlags,
        };

        bindings[binding] = layoutBinding;
        return *this;
    }

    std::unique_ptr<GenDescriptorSetLayout> GenDescriptorSetLayout::Builder::build() const
    {
        return std::make_unique<GenDescriptorSetLayout>(genDevice, bindings);
    }

    // *************** Descriptor Set Layout *********************

    GenDescriptorSetLayout::GenDescriptorSetLayout(
        GenDevice &genDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
        : genDevice{genDevice}, bindings{bindings}
    {
        std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
        for (auto kv : bindings)
        {
            setLayoutBindings.push_back(kv.second);
        }

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<uint32_t>(setLayoutBindings.size()),
            .pBindings = setLayoutBindings.data(),
        };

        if (vkCreateDescriptorSetLayout(
                genDevice.device(),
                &descriptorSetLayoutInfo,
                nullptr,
                &descriptorSetLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    GenDescriptorSetLayout::~GenDescriptorSetLayout()
    {
        vkDestroyDescriptorSetLayout(genDevice.device(), descriptorSetLayout, nullptr);
    }

    // *************** Descriptor Pool Builder *********************

    GenDescriptorPool::Builder &GenDescriptorPool::Builder::addPoolSize(
        VkDescriptorType descriptorType, uint32_t count)
    {
        poolSizes.push_back({descriptorType, count});
        return *this;
    }

    GenDescriptorPool::Builder &GenDescriptorPool::Builder::setPoolFlags(
        VkDescriptorPoolCreateFlags flags)
    {
        poolFlags = flags;
        return *this;
    }
    GenDescriptorPool::Builder &GenDescriptorPool::Builder::setMaxSets(uint32_t count)
    {
        maxSets = count;
        return *this;
    }

    std::unique_ptr<GenDescriptorPool> GenDescriptorPool::Builder::build() const
    {
        return std::make_unique<GenDescriptorPool>(genDevice, maxSets, poolFlags, poolSizes);
    }

    // *************** Descriptor Pool *********************

    GenDescriptorPool::GenDescriptorPool(
        GenDevice &genDevice,
        uint32_t maxSets,
        VkDescriptorPoolCreateFlags poolFlags,
        const std::vector<VkDescriptorPoolSize> &poolSizes) : genDevice{genDevice}
    {
        VkDescriptorPoolCreateInfo descriptorPoolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = poolFlags,
            .maxSets = maxSets,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        if (vkCreateDescriptorPool(genDevice.device(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    GenDescriptorPool::~GenDescriptorPool()
    {
        vkDestroyDescriptorPool(genDevice.device(), descriptorPool, nullptr);
    }

    bool GenDescriptorPool::allocateDescriptor(
        const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const
    {
        VkDescriptorSetAllocateInfo allocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = descriptorPool,
            .descriptorSetCount = 1,
            .pSetLayouts = &descriptorSetLayout,
        };

        // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
        // a new pool whenever an old pool fills up. But this is beyond our current scope
        if (vkAllocateDescriptorSets(genDevice.device(), &allocInfo, &descriptor) != VK_SUCCESS)
        {
            return false;
        }
        return true;
    }

    void GenDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const
    {
        vkFreeDescriptorSets(
            genDevice.device(),
            descriptorPool,
            static_cast<uint32_t>(descriptors.size()),
            descriptors.data());
    }

    void GenDescriptorPool::resetPool()
    {
        vkResetDescriptorPool(genDevice.device(), descriptorPool, 0);
    }

    // *************** Descriptor Writer *********************

    GenDescriptorWriter::GenDescriptorWriter(GenDescriptorSetLayout &setLayout, GenDescriptorPool &pool)
        : setLayout{setLayout}, pool{pool} {}

    GenDescriptorWriter &GenDescriptorWriter::writeBuffer(
        uint32_t binding, VkDescriptorBufferInfo *bufferInfo)
    {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = bindingDescription.descriptorType,
            .pBufferInfo = bufferInfo,
        };

        writes.push_back(write);
        return *this;
    }

    GenDescriptorWriter &GenDescriptorWriter::writeImage(
        uint32_t binding, VkDescriptorImageInfo *imageInfo)
    {
        assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");

        auto &bindingDescription = setLayout.bindings[binding];

        assert(
            bindingDescription.descriptorCount == 1 &&
            "Binding single descriptor info, but binding expects multiple");

        VkWriteDescriptorSet write{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstBinding = binding,
            .descriptorCount = 1,
            .descriptorType = bindingDescription.descriptorType,
            .pImageInfo = imageInfo,
        };

        writes.push_back(write);
        return *this;
    }

    bool GenDescriptorWriter::build(VkDescriptorSet &set)
    {
        bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
        if (!success)
        {
            return false;
        }
        overwrite(set);
        return true;
    }

    void GenDescriptorWriter::overwrite(VkDescriptorSet &set)
    {
        for (auto &write : writes)
        {
            write.dstSet = set;
        }
        vkUpdateDescriptorSets(pool.genDevice.device(), writes.size(), writes.data(), 0, nullptr);
    }

} // namespace gen
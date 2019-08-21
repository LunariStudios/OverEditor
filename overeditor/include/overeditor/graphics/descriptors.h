#ifndef OVEREDITOR_DESCRIPTORS_H
#define OVEREDITOR_DESCRIPTORS_H

#include <vulkan/vulkan.hpp>
#include <vector>
#include <overeditor/utility/memory_layout.h>
#include "device_context.h"

namespace overeditor {

    class DescriptorsController {
    private:
        vk::DescriptorPool descriptorPool;
        std::vector<vk::DescriptorSet> descriptors;
        std::vector<vk::Buffer> descriptorsBuffers;
        std::vector<vk::DeviceMemory> descriptorsMemories;
    public:
        using LayoutPtr =  const DescriptorLayout *;
        using LayoutList = std::vector<LayoutPtr>;

        static DescriptorsController createFor(
                const overeditor::DeviceContext &deviceContext,
                const LayoutList &layouts,
                std::vector<vk::DescriptorSetLayout> vkLayouts
        );

        DescriptorsController() = default;

        DescriptorsController(
                const vk::DescriptorPool &descriptorPool,
                const std::vector<vk::DescriptorSet> &descriptors,
                const std::vector<vk::Buffer> &descriptorsBuffers,
                const std::vector<vk::DeviceMemory> &descriptorsMemories
        ) : descriptorPool(descriptorPool),
            descriptors(descriptors),
            descriptorsBuffers(descriptorsBuffers),
            descriptorsMemories(descriptorsMemories) {

        }

        const vk::DescriptorPool &getDescriptorPool() const {
            return descriptorPool;
        }

        const std::vector<vk::DescriptorSet> &getDescriptors() const {
            return descriptors;
        }

        const std::vector<vk::Buffer> &getDescriptorsBuffers() const {
            return descriptorsBuffers;
        }

        const std::vector<vk::DeviceMemory> &getDescriptorsMemories() const {
            return descriptorsMemories;
        }


    };
}
#endif

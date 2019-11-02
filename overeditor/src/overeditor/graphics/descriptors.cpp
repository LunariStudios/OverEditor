#include <overeditor/graphics/descriptors.h>
#include <overeditor/graphics/device_context.h>
#include <overeditor/utility/vulkan_utility.h>

namespace overeditor {
    void append_sizes_to(
            const overeditor::DescriptorLayout *const &layout,
            std::vector<vk::DescriptorPoolSize> &output
    ) {
        const auto &elements = layout->getElements();
        for (const overeditor::DescriptorElement &element : elements) {
            output.emplace_back(
                    element.getType(),
                    element.getElementCount()
            );
        }
    }

    void append_buffers_to(
            const DescriptorsController::LayoutList &descriptors,
            const overeditor::DeviceContext &dev,
            std::vector<vk::Buffer> &bufs,
            std::vector<vk::DeviceMemory> &memories
    ) {
        const auto &device = dev.getDevice();
        for (const DescriptorsController::LayoutPtr &d : descriptors) {

            for (const auto &e : d->getElements()) {
                if (e.isImported()) {
                    continue;
                }
                vk::Buffer buf;
                vk::DeviceMemory mem;
                overeditor::createBuffer(
                        dev,
                        e.getSize(),
                        buf,
                        mem,
                        vk::BufferUsageFlagBits::eUniformBuffer
                );
                bufs.emplace_back(buf);
                memories.emplace_back(mem);
            }
        }
    }

    std::vector<vk::DescriptorPoolSize> get_sizes(
            const DescriptorsController::LayoutList &layouts
    ) {
        std::vector<vk::DescriptorPoolSize> result;
        for (const DescriptorsController::LayoutPtr &layout : layouts) {
            append_sizes_to(layout, result);
        }
        return result;
    }

    DescriptorsController DescriptorsController::createFor(
            const DeviceContext &deviceContext,
            const LayoutList &layouts,
            std::vector<vk::DescriptorSetLayout> vkLayouts
    ) {
        const auto &device = deviceContext.getDevice();
        vk::DescriptorPool descriptorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
        std::vector<vk::Buffer> buffers;
        std::vector<vk::DeviceMemory> memories;
        if (!layouts.empty()) {
            append_buffers_to(layouts, deviceContext, buffers, memories);
            std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = get_sizes(layouts);

            LOG_INFO << "Sizes " << descriptorPoolSizes.size() << " @ " << sizeof(vk::DescriptorPoolSize);
            descriptorPool = device.createDescriptorPool(
                    vk::DescriptorPoolCreateInfo(
                            (vk::DescriptorPoolCreateFlags) 0,
                            vkLayouts.size(),
                            descriptorPoolSizes.size(),
                            descriptorPoolSizes.data()
                    )
            );
            sets = device.allocateDescriptorSets(
                    vk::DescriptorSetAllocateInfo(
                            descriptorPool,
                            vkLayouts.size(),
                            vkLayouts.data()
                    )
            );
        }
        return DescriptorsController(
                descriptorPool,
                sets,
                buffers,
                memories
        );
    }
}
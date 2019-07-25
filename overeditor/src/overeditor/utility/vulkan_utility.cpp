#include <overeditor/utility/vulkan_utility.h>

namespace overeditor::utility::vulkan {
    using DeviceContext = overeditor::graphics::DeviceContext;

    vk::DeviceMemory setupBufferMemory(
            const DeviceContext &deviceContext,
            vk::Buffer &buffer,
            void *data
    ) {
        auto &device = deviceContext.getDevice();
        auto requirements = device.getBufferMemoryRequirements(buffer);
        size_t size = requirements.size;

        auto mem = device.allocateMemory(
                vk::MemoryAllocateInfo(
                        size,
                        getMemoryTypeIndex(
                                deviceContext,
                                requirements,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
                        )
                )
        );
        device.bindBufferMemory(buffer, mem, 0);
        void *writeAddr;
        device.mapMemory(
                mem, 0, size,
                (vk::MemoryMapFlags) 0,
                &writeAddr
        );
        memcpy(writeAddr, data, size);
        device.unmapMemory(mem);
        //device.flushMappedMemoryRanges({vk::MappedMemoryRange(mem, 0, size)});
        return mem;
    }

    uint32_t getMemoryTypeIndex(
            const DeviceContext &device,
            const vk::MemoryRequirements &requirements,
            const vk::MemoryPropertyFlags &flags
    ) {
        auto &prop = device.getCandidate().getMemoryProperties();
        auto typeFilter = requirements.memoryTypeBits;
        for (uint32_t i = 0; i < prop.memoryTypeCount; ++i) {
            auto &type = prop.memoryTypes[i];
            if (!(typeFilter & (1u << i))) {
                continue;
            }
            if ((type.propertyFlags & flags) != flags) {
                continue;
            }
            return i;
        }
        return -1u;
    }

    vk::DeviceMemory createBufferMemory(const overeditor::graphics::DeviceContext &deviceContext, vk::Buffer &buffer) {
        auto &device = deviceContext.getDevice();
        auto requirements = device.getBufferMemoryRequirements(buffer);
        size_t size = requirements.size;

        auto mem = device.allocateMemory(
                vk::MemoryAllocateInfo(
                        size,
                        getMemoryTypeIndex(
                                deviceContext,
                                requirements,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
                        )
                )
        );
        device.bindBufferMemory(buffer, mem, 0);
        return mem;
    }
}
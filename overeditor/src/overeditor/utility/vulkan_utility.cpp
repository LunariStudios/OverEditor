#include <overeditor/utility/vulkan_utility.h>

namespace overeditor {
    using DeviceContext = overeditor::DeviceContext;

    uint32_t getMemoryTypeIndex(
            const DeviceContext &device,
            const vk::MemoryRequirements &requirements,
            const vk::MemoryPropertyFlags &flags
    ) {
        auto &prop = device.getCandidate().getMemoryProperties();
        auto typeFilter = requirements.memoryTypeBits;
        for (
                uint32_t i = 0; i < prop.memoryTypeCount; ++i
                ) {
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

    void createBuffer(
            const overeditor::DeviceContext &dev,
            size_t size,
            vk::Buffer &buf,
            vk::DeviceMemory &mem,
            vk::BufferUsageFlagBits bufUsage
    ) {
        const auto &device = dev.getDevice();
        buf = device.createBuffer(
                vk::BufferCreateInfo(
                        (vk::BufferCreateFlags) 0,
                        size,
                        bufUsage
                )
        );
        auto requirements = device.getBufferMemoryRequirements(buf);
        size_t s = requirements.size;

        mem = device.allocateMemory(
                vk::MemoryAllocateInfo(
                        s,
                        getMemoryTypeIndex(
                                dev,
                                requirements,
                                vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
                        )
                )
        );
        device.bindBufferMemory(buf, mem, 0);
    }

    void createAndPrepareBuffer(
            const overeditor::DeviceContext &dev,
            size_t size,
            vk::Buffer &buf,
            vk::DeviceMemory &mem,
            vk::BufferUsageFlagBits bufUsage,
            void *data
    ) {
        createBuffer(dev, size, buf, mem, bufUsage);
        const auto &device = dev.getDevice();
        void *writeAddr;
        device.mapMemory(
                mem, 0, size,
                (vk::MemoryMapFlags) 0,
                &writeAddr
        );
        memcpy(writeAddr, data, size);
        device.unmapMemory(mem);
    }



}
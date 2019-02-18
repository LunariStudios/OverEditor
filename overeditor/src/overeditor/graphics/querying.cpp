#include <overeditor/graphics/querying.h>
#include <plog/Log.h>

namespace overeditor::graphics {
    PhysicalDeviceCandidate::PhysicalDeviceCandidate(
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface
    ) : device(device), indices(), score(0), suitable(true),
        deviceProperties(device.getProperties()),
        memoryProperties(device.getMemoryProperties()),
        queueFamilyProperties(device.getQueueFamilyProperties()) {
        auto name = deviceProperties.deviceName;
        auto type = deviceProperties.deviceType;
        if (type == vk::PhysicalDeviceType::eCpu || type == vk::PhysicalDeviceType::eOther) {
            suitable = false;
        }
        size_t totalMemory = 0;
        for (int j = 0; j < memoryProperties.memoryHeapCount; ++j) {
            const vk::MemoryHeap &heap = memoryProperties.memoryHeaps[j];
            if ((heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) == vk::MemoryHeapFlagBits::eDeviceLocal) {
                size_t mem = heap.size;
                totalMemory += mem;
            }
        }
        score += (uint32_t) (BYTES_TO_GIB(totalMemory) * MEMORY_SCORE_MULTIPLIER);
        auto properties = device.getQueueFamilyProperties();
        VkBool32 hasSurfaceExtension = VK_FALSE;
        for (uint32_t i = 0; i < properties.size(); ++i) {
            vk::QueueFamilyProperties &prop = properties[i];
            auto a = prop.queueFlags & vk::QueueFlagBits::eCompute;
            indices.offer(i, prop);
            if (!hasSurfaceExtension) {
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &hasSurfaceExtension);
            }
        }
        if (!hasSurfaceExtension) {
            suitable = false;
        }
    }

    const vk::PhysicalDevice &PhysicalDeviceCandidate::getDevice() const {
        return device;
    }

    const QueueFamilyIndices &PhysicalDeviceCandidate::getIndices() const {
        return indices;
    }

    uint32_t PhysicalDeviceCandidate::getScore() const {
        return score;
    }

    bool PhysicalDeviceCandidate::isSuitable() const {
        return suitable;
    }

    const vk::PhysicalDeviceProperties &PhysicalDeviceCandidate::getDeviceProperties() const {
        return deviceProperties;
    }

    const vk::PhysicalDeviceMemoryProperties &PhysicalDeviceCandidate::getMemoryProperties() const {
        return memoryProperties;
    }

    const std::vector<vk::QueueFamilyProperties> &PhysicalDeviceCandidate::getQueueFamilyProperties() const {
        return queueFamilyProperties;
    }

    const std::string PhysicalDeviceCandidate::getName() const {
        return std::string(deviceProperties.deviceName);
    }
}
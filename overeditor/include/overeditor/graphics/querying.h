#ifndef OVEREDITOR_QUERYING_H
#define OVEREDITOR_QUERYING_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/utility/string_utility.h>
#include <plog/Record.h>

namespace overeditor::graphics {

#define MEMORY_SCORE_MULTIPLIER 1

#define QUEUE_FAMILY_BIT_LOG(bit, name, flags) INDENTATION(3) << "* " << name << ": " << ((flags & bit) == (vk::QueueFlags) bit ? "present" : "absent")

    class PhysicalDeviceCandidate {
    private:
        vk::PhysicalDevice device;
        QueueFamilyIndices indices;
        uint32_t score;
        bool suitable;
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    public:
        PhysicalDeviceCandidate(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

        const vk::PhysicalDevice &getDevice() const;

        const QueueFamilyIndices &getIndices() const;

        uint32_t getScore() const;

        bool isSuitable() const;

        const std::string getName() const {
            return std::string(deviceProperties.deviceName);
        }

        const vk::PhysicalDeviceProperties &getDeviceProperties() const;

        const vk::PhysicalDeviceMemoryProperties &getMemoryProperties() const;

        const std::vector<vk::QueueFamilyProperties> &getQueueFamilyProperties() const;
    };
}
#endif
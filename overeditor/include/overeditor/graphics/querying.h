#ifndef OVEREDITOR_QUERYING_H
#define OVEREDITOR_QUERYING_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/graphics/requirements.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/success_status.h>
#include <plog/Record.h>

namespace overeditor::graphics {

#define MEMORY_SCORE_MULTIPLIER 100

#define LOG_QUEUE_FAMILY_BIT(bit, name, flags) INDENTATION(3) << "* " << name << ": " << ((flags & bit) == (vk::QueueFlags) bit ? "present" : "absent")

    class PhysicalDeviceCandidate {
    private:
        vk::PhysicalDevice device;
        QueueFamilyIndices indices;
        uint32_t score;
        overeditor::utility::SuccessStatus suitableness;
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
    public:
        PhysicalDeviceCandidate(const overeditor::graphics::Requirements &requirements,
                                const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

        const vk::PhysicalDevice &getDevice() const;

        const QueueFamilyIndices &getIndices() const;

        uint32_t getScore() const;

        const utility::SuccessStatus &getSuitableness() const;

        const std::string getName() const;

        const vk::PhysicalDeviceProperties &getDeviceProperties() const;

        const vk::PhysicalDeviceMemoryProperties &getMemoryProperties() const;

        const std::vector<vk::QueueFamilyProperties> &getQueueFamilyProperties() const;
    };

}
#endif
#ifndef OVEREDITOR_QUEUEFAMILIES_H
#define OVEREDITOR_QUEUEFAMILIES_H

#include <optional>
#include <vulkan/vulkan.hpp>
#include <ostream>

namespace overeditor::graphics {
    class QueueFamily {
    private:
        vk::QueueFlagBits bit;
        std::optional<uint32_t> index;
    public:
        explicit QueueFamily(vk::QueueFlagBits bit);

        void offer(uint32_t index, const vk::QueueFamilyProperties &properties);

        bool tryGet(uint32_t *result) const;

        friend std::ostream &operator<<(std::ostream &os, const QueueFamily &family);
    };

    class QueueFamilyIndices {
    private:
        QueueFamily graphics;
    public:
        QueueFamilyIndices();

        void offer(uint32_t index, const vk::QueueFamilyProperties &properties);

        const QueueFamily &getGraphics() const;
    };
}
#endif

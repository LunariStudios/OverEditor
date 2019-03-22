#ifndef OVEREDITOR_QUEUEFAMILIES_H
#define OVEREDITOR_QUEUEFAMILIES_H

#include <optional>
#include <vulkan/vulkan.hpp>
#include <ostream>
#include <functional>

namespace overeditor::graphics {
    class QueueFamily {
    protected:
        std::optional<uint32_t> index;
    public:
        explicit QueueFamily();

        virtual void offer(uint32_t index, const vk::QueueFamilyProperties &properties,
                           const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface) = 0;

        bool tryGet(uint32_t *result) const;

        bool present() const;

        friend std::ostream &operator<<(std::ostream &os, const QueueFamily &family);
    };

    class FlagBitQueueFamily : public QueueFamily {
    private:
        vk::QueueFlagBits bit;
    public:
        explicit FlagBitQueueFamily(vk::QueueFlagBits bit);

        void offer(
                uint32_t index,
                const vk::QueueFamilyProperties &properties,
                const vk::PhysicalDevice &device,
                const vk::SurfaceKHR &surface
        ) override;
    };

    class PresentationQueueFamily : public QueueFamily {
    private:
        uint32_t lastScore;
    public:
        PresentationQueueFamily();

        void offer(
                uint32_t index,
                const vk::QueueFamilyProperties &properties,
                const vk::PhysicalDevice &device,
                const vk::SurfaceKHR &surface
        ) override;
    };

    class QueueFamilyIndices {
    private:
        FlagBitQueueFamily graphics;
        PresentationQueueFamily presentation;
    public:
        QueueFamilyIndices();

        void offer(
                uint32_t index,
                const vk::QueueFamilyProperties &properties,
                const vk::PhysicalDevice &device,
                const vk::SurfaceKHR &surface
        );

        const QueueFamily &getGraphics() const;

        const PresentationQueueFamily &getPresentation() const;
    };
}
#endif

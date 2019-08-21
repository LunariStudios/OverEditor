#include <overeditor/graphics/queue_families.h>

namespace overeditor {
    QueueFamily::QueueFamily() : index() {}

    bool QueueFamily::tryGet(uint32_t *result) const {
        if (index) {
            *result = index.value();
            return true;
        }
        return false;
    }


    std::ostream &operator<<(std::ostream &os, const QueueFamily &family) {
        os << (family.index ? "available" : "absent");
        if (family.index) {
            os << " @ " << family.index.value();
        }
        return os;
    }


    void QueueFamilyIndices::offer(
            uint32_t index,
            const vk::QueueFamilyProperties &properties,
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface
    ) {
        graphics.offer(index, properties, device, surface);
        presentation.offer(index, properties, device, surface);
    }

    QueueFamilyIndices::QueueFamilyIndices() : graphics(vk::QueueFlagBits::eGraphics) {}

    const QueueFamily &QueueFamilyIndices::getGraphics() const {
        return graphics;
    }

    const PresentationQueueFamily &QueueFamilyIndices::getPresentation() const {
        return presentation;
    }


    void FlagBitQueueFamily::offer(
            uint32_t index,
            const vk::QueueFamilyProperties &properties,
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface
    ) {
        if (!FlagBitQueueFamily::index && (properties.queueFlags & bit) == bit) {
            FlagBitQueueFamily::index = index;
        }
    }

    bool QueueFamily::present() const {
        return (bool) index;
    }

    uint32_t QueueFamily::get() const {
        uint32_t result;
        if (!tryGet(&result)) {
            throw std::runtime_error("Unable to get index.");
        }
        return result;
    }

    FlagBitQueueFamily::FlagBitQueueFamily(vk::QueueFlagBits bit) : bit(bit) {}

    void PresentationQueueFamily::offer(
            uint32_t index,
            const vk::QueueFamilyProperties &properties,
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface
    ) {

        VkBool32 supported;
        vkGetPhysicalDeviceSurfaceSupportKHR((VkPhysicalDevice) device, index, (VkSurfaceKHR) surface, &supported);
        if (supported) {
            QueueFamily::index = index;
        }
    }

    PresentationQueueFamily::PresentationQueueFamily() : lastScore(0) {

    }
}
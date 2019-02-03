#include <overeditor/graphics/queue_families.h>

namespace overeditor::graphics {
    QueueFamily::QueueFamily(vk::QueueFlagBits bit) : bit(bit) {}

    bool QueueFamily::tryGet(uint32_t *result) const {
        if (index) {
            *result = index.value();
            return true;
        }
        return false;
    }

    void QueueFamily::offer(uint32_t index, const vk::QueueFamilyProperties &properties) {
        if (!QueueFamily::index && (properties.queueFlags & bit) == bit) {
            QueueFamily::index = index;
        }
    }

    std::ostream &operator<<(std::ostream &os, const QueueFamily &family) {
        os << (family.index ? "available" : "absent");
        if (family.index) {
            os << " @ " << family.index.value();
        }
        return os;
    }

    void QueueFamilyIndices::offer(uint32_t index, const vk::QueueFamilyProperties &properties) {
        graphics.offer(index, properties);
    }

    QueueFamilyIndices::QueueFamilyIndices() : graphics(vk::QueueFlagBits::eGraphics) {}

    const QueueFamily &QueueFamilyIndices::getGraphics() const {
        return graphics;
    }


}
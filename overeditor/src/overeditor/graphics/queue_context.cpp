#include <overeditor/graphics/queue_context.h>

namespace overeditor::graphics {

    QueueContext::QueueContext(
            const vk::Device &device,
            const QueueFamilyIndices &qIndices,
            uint32_t graphicsIndex,
            uint32_t presentationIndex
    ) {
        familyIndices = qIndices;
        graphicsQueue = device.getQueue(graphicsIndex, 0);
        presentationQueue = device.getQueue(presentationIndex, 0);
    }

    const QueueFamilyIndices &QueueContext::getFamilyIndices() const {
        return familyIndices;
    }

    const vk::Queue &QueueContext::getGraphicsQueue() const {
        return graphicsQueue;
    }

    const vk::Queue &QueueContext::getPresentationQueue() const {
        return presentationQueue;
    }
}
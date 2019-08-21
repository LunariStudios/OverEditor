//
// Created by bruno on 3/25/2019.
//

#ifndef OVEREDITOR_QUEUE_CONTEXT_H
#define OVEREDITOR_QUEUE_CONTEXT_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/queue_families.h>

namespace overeditor {
    class QueueContext {
    private:
        QueueFamilyIndices familyIndices;
        vk::Queue graphicsQueue, presentationQueue;
    public:
        QueueContext(
                const vk::Device &device,
                const QueueFamilyIndices &qIndices,
                uint32_t graphicsIndex,
                uint32_t presentationIndex
        );

        const QueueFamilyIndices &getFamilyIndices() const;

        const vk::Queue &getGraphicsQueue() const;

        const vk::Queue &getPresentationQueue() const;
    };
}
#endif //OVEREDITOR_QUEUE_CONTEXT_H

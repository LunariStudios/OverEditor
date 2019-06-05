#ifndef OVEREDITOR_DEVICE_CONTEXT_H
#define OVEREDITOR_DEVICE_CONTEXT_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/swapchain_context.h>
#include <overeditor/graphics/queue_context.h>

namespace overeditor::graphics {


    class DeviceContext {
    private:
        QueueContext *queueContext;
        SwapChainContext *swapChainContext;
        vk::Device device;
    public:
        DeviceContext(
                const PhysicalDeviceCandidate &dev,
                const Requirements &requirements,
                const vk::SurfaceKHR &surface
        );

        ~DeviceContext();

        QueueContext *getQueueContext() const;

        SwapChainContext *getSwapChainContext() const;

        const vk::Device &getDevice() const;
    };
}
#endif

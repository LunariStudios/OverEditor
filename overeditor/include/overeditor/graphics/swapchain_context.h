#ifndef OVEREDITOR_SWAPCHAIN_CONTEXT_H
#define OVEREDITOR_SWAPCHAIN_CONTEXT_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/querying.h>

namespace overeditor::graphics {
    /**
     * Holds all the information about an Application's Vulkan SwapChain
     */
    class SwapChainContext {
    private:
        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchainImages;
        vk::Format swapchainFormat;
        vk::Extent2D swapchainExtent;
        const vk::Device *devicePtr;
    public:

        SwapChainContext(
                const vk::Device &device,
                const overeditor::graphics::QueueFamilyIndices &qIndices,
                const overeditor::graphics::SwapchainSupportDetails &scSupport,
                const vk::SurfaceKHR &surface
        );

        ~SwapChainContext();

        const vk::SwapchainKHR &getSwapchain() const;

        const std::vector<vk::Image> &getSwapchainImages() const;

        vk::Format getSwapchainFormat() const;

        const vk::Extent2D &getSwapchainExtent() const;

        const vk::Device *getDevicePtr() const;

    };
}
#endif

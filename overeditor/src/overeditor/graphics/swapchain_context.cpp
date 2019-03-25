#include <overeditor/graphics/swapchain_context.h>

namespace overeditor::graphics {
    SwapChainContext::SwapChainContext(
            const vk::Device &device,
            const overeditor::graphics::QueueFamilyIndices &qIndices,
            const overeditor::graphics::SwapchainSupportDetails &scSupport,
            const vk::SurfaceKHR &surface
    ) : devicePtr(&device) {
        vk::SurfaceFormatKHR surfaceFormat = scSupport.selectSurfaceFormat();
        vk::PresentModeKHR presentMode = scSupport.selectPresentMode();
        vk::Extent2D extent = scSupport.selectSwapExtent();
        const auto &surfaceCapabilities = scSupport.getSurfaceCapabilities();
        uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
        auto info = vk::SwapchainCreateInfoKHR(
                (vk::SwapchainCreateFlagsKHR) 0, // Flags
                surface, // Surface
                imageCount, // minImageCount
                surfaceFormat.format, // imageFormat
                surfaceFormat.colorSpace, // imageColorSpace
                extent, // imageExtent
                1, // imageArrayLayers
                vk::ImageUsageFlagBits::eColorAttachment // imageUsage
        );
        uint32_t graphicsIndex, presentIndex;
        if (!qIndices.getGraphics().tryGet(&graphicsIndex)) {
            throw std::runtime_error("Unable to get graphics queue index.");
        }
        if (!qIndices.getPresentation().tryGet(&presentIndex)) {
            throw std::runtime_error("Unable to get presentation queue index.");
        }

        // TODO: Find way to do this without allocating from the heap
        // This exists because once we exit the scope, graphicsIndex and presentIndex gets destroyed
        auto *queueFamilyIndices = new uint32_t[2]{graphicsIndex, presentIndex};
        if (graphicsIndex != presentIndex) {
            info.imageSharingMode = vk::SharingMode::eConcurrent;
            info.queueFamilyIndexCount = 2;
            info.pQueueFamilyIndices = queueFamilyIndices;
        } else {
            info.imageSharingMode = vk::SharingMode::eExclusive;
            info.queueFamilyIndexCount = 0; // Optional
            info.pQueueFamilyIndices = nullptr; // Optional
        }
        info.preTransform = surfaceCapabilities.currentTransform;
        info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        LOG_VECTOR_WITH("Surface formats", scSupport.getSurfaceFormats(), 1,
                        "Format: " << vk::to_string(value.format) << ", color space: "
                                   << vk::to_string(value.colorSpace));
        LOG_VECTOR_WITH("Presentation modes", scSupport.getPresentModes(), 1, vk::to_string(value));

        swapchainFormat = info.imageFormat;
        swapchainExtent = info.imageExtent;
        swapchain = device.createSwapchainKHR(info);
        // Free memory used on createSwapchainInfo
        delete info.pQueueFamilyIndices;
        uint32_t imgCount;
        vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr);
        swapchainImages.resize(imgCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imgCount, reinterpret_cast<VkImage *>(swapchainImages.data()));

    }

    SwapChainContext::~SwapChainContext() {
        vkDestroySwapchainKHR(*devicePtr, swapchain, nullptr);
    }

    const vk::SwapchainKHR &SwapChainContext::getSwapchain() const {
        return swapchain;
    }

    const std::vector<vk::Image> &SwapChainContext::getSwapchainImages() const {
        return swapchainImages;
    }

    vk::Format SwapChainContext::getSwapchainFormat() const {
        return swapchainFormat;
    }

    const vk::Extent2D &SwapChainContext::getSwapchainExtent() const {
        return swapchainExtent;
    }

    const vk::Device *SwapChainContext::getDevicePtr() const {
        return devicePtr;
    }
}
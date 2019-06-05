#include <overeditor/graphics/swapchain_context.h>

namespace overeditor::graphics {
    SwapChainContext::SwapChainContext(
            const vk::Device &device,
            const overeditor::graphics::QueueFamilyIndices &qIndices,
            const overeditor::graphics::SwapchainSupportDetails &scSupport,
            const vk::SurfaceKHR &surface
    ) : devicePtr(&device), swapchainImages() {
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

        uint32_t queueFamilyIndices[2] = {graphicsIndex, presentIndex};
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

        swapchainFormat = info.imageFormat;
        swapchainExtent = info.imageExtent;
        swapchain = device.createSwapchainKHR(info);
        uint32_t imgCount;
        std::vector<vk::Image> images;
        vkGetSwapchainImagesKHR(device, swapchain, &imgCount, nullptr);
        images.resize(imgCount);
        vkGetSwapchainImagesKHR(device, swapchain, &imgCount, reinterpret_cast<VkImage *>(images.data()));
        for (vk::Image &img : images) {
            vk::ImageViewCreateInfo info(
                    (vk::ImageViewCreateFlags) 0, // Flags
                    img, // Image
                    vk::ImageViewType::e2D, // Image Type
                    swapchainFormat, //Image format
                    vk::ComponentMapping(), // Content Mapping
                    vk::ImageSubresourceRange(
                            vk::ImageAspectFlagBits::eColor, // Color bit
                            0, // Base Mip Level
                            1, // Level Count
                            0, // Base Array Layer
                            1 // Layer count
                    )
            );
            vk::ImageView view;
            auto createImgViewResult = vkCreateImageView(
                    device,
                    reinterpret_cast<VkImageViewCreateInfo *>(&info),
                    nullptr,
                    reinterpret_cast<VkImageView *>(&view)
            );
            if (createImgViewResult != VK_SUCCESS) {
                throw std::runtime_error(
                        std::string("Unable to create image view: ") + vk::to_string((vk::Result) createImgViewResult));
            }
            swapchainImages.emplace_back(img, view);
        }
    }

    SwapChainContext::~SwapChainContext() {
        const vk::Device &device = *devicePtr;
        for (ImageContext &img : swapchainImages) {
            vkDestroyImageView(device, img.getView(), nullptr);
        }
        swapchainImages.clear();
        vkDestroySwapchainKHR(*devicePtr, swapchain, nullptr);
    }

    const vk::SwapchainKHR &SwapChainContext::getSwapchain() const {
        return swapchain;
    }

    const std::vector<ImageContext> &SwapChainContext::getSwapchainImages() const {
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
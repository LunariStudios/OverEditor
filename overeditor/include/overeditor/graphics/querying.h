#ifndef OVEREDITOR_QUERYING_H
#define OVEREDITOR_QUERYING_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/graphics/requirements.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/success_status.h>
#include <overeditor/utility/vulkan_utility.h>
#include <plog/Record.h>

namespace overeditor::graphics {

#define MEMORY_SCORE_MULTIPLIER 100

#define LOG_QUEUE_FAMILY_BIT(bit, name, flags) INDENTATION(3) << "* " << name << ": " << ((flags & bit) == (vk::QueueFlags) bit ? "present" : "absent")

    class SwapchainSupportDetails {
    private:
        vk::SurfaceCapabilitiesKHR surfaceCapabilities;
        std::vector<vk::SurfaceFormatKHR> surfaceFormats;
        std::vector<vk::PresentModeKHR> presentModes;
    public:
        SwapchainSupportDetails(
                const vk::PhysicalDevice &device,
                const vk::SurfaceKHR &surface
        );

        const vk::SurfaceCapabilitiesKHR &getSurfaceCapabilities() const;

        const std::vector<vk::SurfaceFormatKHR> &getSurfaceFormats() const;

        const std::vector<vk::PresentModeKHR> &getPresentModes() const;

        // this is inline as a hack because queueFamiliesIndices is on the stack and gets fucked up once we leave
        // the scope of the function
        inline const vk::SwapchainCreateInfoKHR createSwapchainInfo(
                const QueueFamilyIndices &indices,
                const vk::SurfaceKHR &surface
        ) const {
            vk::SurfaceFormatKHR surfaceFormat = selectSurfaceFormat();
            vk::PresentModeKHR presentMode = selectPresentMode();
            vk::Extent2D extent = chooseSwapExtent();
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
            if (!indices.getGraphics().tryGet(&graphicsIndex)) {
                throw std::runtime_error("Unable to get graphics queue index.");
            }
            if (!indices.getPresentation().tryGet(&presentIndex)) {
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
            return info;
        }


    private:
        vk::PresentModeKHR selectPresentMode() const {
            if (presentModes.empty()) {
                throw std::runtime_error("There are no surface formats available");
            }

            for (const auto &availablePresentMode : presentModes) {
                if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
                    return availablePresentMode;
                }
            }
            return vk::PresentModeKHR::eFifo;
        }

        const vk::SurfaceFormatKHR selectSurfaceFormat() const {
            if (surfaceFormats.empty()) {
                throw std::runtime_error("There are no surface formats available");
            }
            if (surfaceFormats.size() == 1 && surfaceFormats[0].format == vk::Format::eUndefined) {
                auto surfaceFormat = vk::SurfaceFormatKHR();
                surfaceFormat.format = vk::Format::eB8G8R8A8Unorm;
                surfaceFormat.colorSpace = vk::ColorSpaceKHR::eSrgbNonlinear;
                return surfaceFormat;
            } else {
                return *surfaceFormats.begin();
            }
        }

        VkExtent2D chooseSwapExtent() const {
            if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return surfaceCapabilities.currentExtent;
            } else {
                VkExtent2D actualExtent = {PREFERRED_WIDTH, PREFERRED_HEIGHT};

                actualExtent.width = std::max(
                        surfaceCapabilities.minImageExtent.width,
                        std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width)
                );
                actualExtent.height = std::max(
                        surfaceCapabilities.minImageExtent.height,
                        std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

                return actualExtent;
            }
        }
    };

    class PhysicalDeviceCandidate {
    private:
        vk::PhysicalDevice device;
        QueueFamilyIndices indices;
        uint32_t score;
        overeditor::utility::SuccessStatus suitableness;
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
        SwapchainSupportDetails swapchainSupportDetails;
    public:
        PhysicalDeviceCandidate(const overeditor::graphics::Requirements &requirements,
                                const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface);

        const vk::PhysicalDevice &getDevice() const;

        const QueueFamilyIndices &getIndices() const;

        uint32_t getScore() const;

        const utility::SuccessStatus &getSuitableness() const;

        const std::string getName() const;

        const vk::PhysicalDeviceProperties &getDeviceProperties() const;

        const vk::PhysicalDeviceMemoryProperties &getMemoryProperties() const;

        const std::vector<vk::QueueFamilyProperties> &getQueueFamilyProperties() const;

        const SwapchainSupportDetails &getSwapchainSupportDetails() const;
    };


}
#endif
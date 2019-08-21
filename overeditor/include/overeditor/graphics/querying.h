#ifndef OVEREDITOR_QUERYING_H
#define OVEREDITOR_QUERYING_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/graphics/requirements.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/success_status.h>
#include <plog/Record.h>

namespace overeditor {

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

        vk::Extent2D selectSwapExtent() const {
            if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
                return surfaceCapabilities.currentExtent;
            } else {
                auto w = std::clamp(
                        (uint32_t) PREFERRED_WIDTH,
                        surfaceCapabilities.minImageExtent.width,
                        surfaceCapabilities.maxImageExtent.width
                );
                auto h = std::clamp(
                        (uint32_t) PREFERRED_HEIGHT,
                        surfaceCapabilities.minImageExtent.height,
                        surfaceCapabilities.maxImageExtent.height
                );
                return vk::Extent2D(w, h);
            }
        }
    };

    class PhysicalDeviceCandidate {
    private:
        vk::PhysicalDevice device;
        QueueFamilyIndices indices;
        uint32_t score;
        overeditor::SuccessStatus suitableness;
        vk::PhysicalDeviceProperties deviceProperties;
        vk::PhysicalDeviceMemoryProperties memoryProperties;
        std::vector<vk::QueueFamilyProperties> queueFamilyProperties;
        SwapchainSupportDetails swapchainSupportDetails;
    public:
        PhysicalDeviceCandidate(
                const overeditor::Requirements &requirements,
                const vk::PhysicalDevice &device,
                const vk::SurfaceKHR &surface
        );

        const vk::PhysicalDevice &getDevice() const;

        const QueueFamilyIndices &getIndices() const;

        uint32_t getScore() const;

        const SuccessStatus &getSuitableness() const;

        const std::string getName() const;

        const vk::PhysicalDeviceProperties &getDeviceProperties() const;

        const vk::PhysicalDeviceMemoryProperties &getMemoryProperties() const;

        const std::vector<vk::QueueFamilyProperties> &getQueueFamilyProperties() const;

        const SwapchainSupportDetails &getSwapchainSupportDetails() const;
    };


}
#endif
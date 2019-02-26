#ifndef OVEREDITOR_VULKAN_UTILITY_H
#define OVEREDITOR_VULKAN_UTILITY_H

#include <vector>
#include <vulkan/vulkan.hpp>

namespace overeditor::utility::vk_utility {


    void enumerateSurfaceFormatsInto(
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface,
            std::vector<vk::SurfaceFormatKHR> &formats
    );

    void enumeratePresentModesInto(
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface,
            std::vector<vk::PresentModeKHR> &formats
    );
}
#endif

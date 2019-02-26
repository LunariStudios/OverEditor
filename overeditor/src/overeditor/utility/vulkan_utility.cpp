#include <overeditor/utility/vulkan_utility.h>

#define ENUMERATE_FUNC(func, type, vec, ...) uint32_t total;\
    func(__VA_ARGS__, &total, nullptr);\
    vec.resize(total);\
    func(device, surface, &total,reinterpret_cast<type *> (vec.data()));\


namespace overeditor::utility::vk_utility {
    void enumerateSurfaceFormatsInto(
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface,
            std::vector<vk::SurfaceFormatKHR> &formats
    ) {
        ENUMERATE_FUNC(vkGetPhysicalDeviceSurfaceFormatsKHR, VkSurfaceFormatKHR, formats, device, surface);
    }

    void enumeratePresentModesInto(
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface,
            std::vector<vk::PresentModeKHR> &formats
    ) {
        ENUMERATE_FUNC(vkGetPhysicalDeviceSurfacePresentModesKHR, VkPresentModeKHR, formats, device, surface);
    }
}
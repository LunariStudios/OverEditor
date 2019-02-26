#ifndef OVEREDITOR_OVEREDITOR_CONSTANTS_H
#define OVEREDITOR_OVEREDITOR_CONSTANTS_H

#include <vector>
#include <vulkan/vulkan.h>
#define PREFERRED_WIDTH 1920
#define PREFERRED_HEIGHT 1080
static std::vector<const char *> kRequiredInstanceExtensions = {
        // VK_KHR_SURFACE_EXTENSION_NAME  - Not included because is already included throught GLFW
};
static std::vector<const char *> kRequiredInstanceLayers = {
        "VK_LAYER_LUNARG_standard_validation"
};
static std::vector<const char *> kRequiredDeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
static std::vector<const char *> kRequiredDeviceLayers = {
};
#endif

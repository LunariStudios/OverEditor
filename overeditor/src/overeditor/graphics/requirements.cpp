#include <overeditor/graphics/requirements.h>

namespace overeditor {
    const std::vector<const char *> &Requirements::getRequiredExtensions() const {
        return requiredExtensions;
    }

    const std::vector<const char *> &Requirements::getRequiredLayers() const {
        return requiredLayers;
    }

    Requirements::Requirements(
            const std::vector<const char *> &requiredExtensions,
            const std::vector<const char *> &requiredLayers
    ) : requiredExtensions(requiredExtensions), requiredLayers(requiredLayers) {}

    void Requirements::checkRequirements(
            const std::vector<vk::ExtensionProperties> &extensions,
            const std::vector<vk::LayerProperties> &layers,
            overeditor::SuccessStatus &status
    ) const {
        for (const char *requirement : requiredExtensions) {
            if (std::none_of(extensions.begin(), extensions.end(), [&](const vk::ExtensionProperties &other) {
                return strcmp(requirement, other.extensionName) == 0;
            })) {
                status.addError(std::string("Missing component: ") + requirement);
            }
        }
        for (const char *requirement : requiredLayers) {
            if (std::none_of(layers.begin(), layers.end(), [&](const vk::LayerProperties &other) {
                return strcmp(requirement, other.layerName) == 0;
            })) {
                status.addError(std::string("Missing component: ") + requirement);
            }
        }

    }

    const Requirements &VulkanRequirements::getDeviceRequirements() const {
        return deviceRequirements;
    }

    const Requirements &VulkanRequirements::getInstanceRequirements() const {
        return instanceRequirements;
    }

    VulkanRequirements::VulkanRequirements(
            const Requirements &deviceRequirements,
            const Requirements &instanceRequirements
    ) : deviceRequirements(deviceRequirements), instanceRequirements(instanceRequirements) {}

    VulkanRequirements VulkanRequirements::createOverEditorRequirements() {
        // Instance
        std::vector<const char *> instanceExtensions, instanceLayers;
        overeditor::add_range(kRequiredInstanceLayers, instanceLayers);
        overeditor::add_range(kRequiredInstanceExtensions, instanceExtensions);
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        instanceExtensions.reserve(glfwExtensionCount);
        for (int l = 0; l < glfwExtensionCount; ++l) {
            instanceExtensions.push_back(glfwExtensions[l]);
        }

        // Device
        std::vector<const char *> deviceExtensions, deviceLayers;
        overeditor::add_range(kRequiredDeviceLayers, deviceLayers);
        overeditor::add_range(kRequiredDeviceExtensions, deviceExtensions);
        return VulkanRequirements(
                Requirements(deviceExtensions, deviceLayers),
                Requirements(instanceExtensions, instanceLayers)
        );

    }
}
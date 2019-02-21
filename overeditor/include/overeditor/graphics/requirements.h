#ifndef OVEREDITOR_REQUIREMENTS_H
#define OVEREDITOR_REQUIREMENTS_H

#include <vector>
#include <GLFW/glfw3.h>
#include <overeditor/overeditor_constants.h>
#include <overeditor/utility/collection_utility.h>
#include <overeditor/utility/success_status.h>
#include <vulkan/vulkan.hpp>
#include <plog/Log.h>

namespace overeditor::graphics {
    class Requirements {
    private:
        std::vector<const char *> requiredExtensions, requiredLayers;
    public:
        Requirements(
                const std::vector<const char *> &requiredExtensions,
                const std::vector<const char *> &requiredLayers
        );

        const std::vector<const char *> &getRequiredExtensions() const;

        const std::vector<const char *> &getRequiredLayers() const;

        void checkRequirements(
                const std::vector<vk::ExtensionProperties> &extensions,
                const std::vector<vk::LayerProperties> &layers,
                overeditor::utility::SuccessStatus &status
        ) const;
    };

    class VulkanRequirements {
    private:
        Requirements deviceRequirements, instanceRequirements;
    public:
        VulkanRequirements(const Requirements &deviceRequirements, const Requirements &instanceRequirements);

        static VulkanRequirements createOverEditorRequirements();;

        const Requirements &getDeviceRequirements() const;

        const Requirements &getInstanceRequirements() const;
    };
}
#endif

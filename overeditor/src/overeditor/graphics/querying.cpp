#include <overeditor/graphics/querying.h>
#include <plog/Log.h>

#define DISCRETE_GPU_BONUS 1500
namespace overeditor {
    PhysicalDeviceCandidate::PhysicalDeviceCandidate(
            const overeditor::Requirements &requirementss,
            const vk::PhysicalDevice &device,
            const vk::SurfaceKHR &surface
    ) : device(device), indices(), score(0), suitableness(),
        deviceProperties(device.getProperties()),
        memoryProperties(device.getMemoryProperties()),
        queueFamilyProperties(device.getQueueFamilyProperties()),
        swapchainSupportDetails(device, surface) {
        auto name = deviceProperties.deviceName;
        auto type = deviceProperties.deviceType;
        if (type == vk::PhysicalDeviceType::eDiscreteGpu) {
            score += DISCRETE_GPU_BONUS;
        }
        size_t totalMemory = 0;
        for (int j = 0; j < memoryProperties.memoryHeapCount; ++j) {
            const vk::MemoryHeap &heap = memoryProperties.memoryHeaps[j];
            if ((heap.flags & vk::MemoryHeapFlagBits::eDeviceLocal) == vk::MemoryHeapFlagBits::eDeviceLocal) {
                size_t mem = heap.size;
                totalMemory += mem;
            }
        }
        score += (uint32_t) (BYTES_TO_GIB(totalMemory) * MEMORY_SCORE_MULTIPLIER);
        auto properties = device.getQueueFamilyProperties();
        for (uint32_t i = 0; i < properties.size(); ++i) {
            vk::QueueFamilyProperties &prop = properties[i];
            auto a = prop.queueFlags & vk::QueueFlagBits::eCompute;
            indices.offer(i, prop, device, surface);
        }
        if (!indices.getPresentation().present() || !indices.getGraphics().present()) {
            suitableness.addError("Presentation queue not supported");
        }

        uint32_t count;
        vkEnumerateDeviceExtensionProperties((VkPhysicalDevice) device, nullptr, &count, nullptr);
        std::vector<vk::ExtensionProperties> extensions(count);
        vkEnumerateDeviceExtensionProperties((VkPhysicalDevice) device, nullptr, &count,
                                             reinterpret_cast<VkExtensionProperties *>(extensions.data()));


        for (const char *requiredExtension : requirementss.getRequiredExtensions()) {
            if (std::none_of(extensions.begin(), extensions.end(), [&](const vk::ExtensionProperties &property) {
                return strcmp(property.extensionName, requiredExtension) == 0;
            })) {
                suitableness.addError(std::string("Required extension not present: ") + requiredExtension);
                break;
            }

        }
        uint32_t layerCount;
        vkEnumerateDeviceLayerProperties((VkPhysicalDevice) device, &layerCount, nullptr);
        std::vector<vk::LayerProperties> layers(layerCount);
        vkEnumerateDeviceLayerProperties((VkPhysicalDevice) device, &layerCount,
                                         reinterpret_cast<VkLayerProperties *>(layers.data()));
        for (const char *requiredLayer : requirementss.getRequiredLayers()) {
            if (std::none_of(layers.begin(), layers.end(), [&](const vk::LayerProperties &property) {
                return strcmp(property.layerName, requiredLayer) == 0;
            })) {
                suitableness.addError(std::string("Required layer not present: ") + requiredLayer);
                break;
            }
        }
        if (swapchainSupportDetails.getSurfaceFormats().empty()) {
            suitableness.addError("There are no surface formats available");

        }
        if (swapchainSupportDetails.getPresentModes().empty()) {
            suitableness.addError("There are no presentation modes available");
        }
    }

    const vk::PhysicalDevice &PhysicalDeviceCandidate::getDevice() const {
        return device;
    }

    const QueueFamilyIndices &PhysicalDeviceCandidate::getIndices() const {
        return indices;
    }

    uint32_t PhysicalDeviceCandidate::getScore() const {
        return score;
    }

    const vk::PhysicalDeviceProperties &PhysicalDeviceCandidate::getDeviceProperties() const {
        return deviceProperties;
    }

    const vk::PhysicalDeviceMemoryProperties &PhysicalDeviceCandidate::getMemoryProperties() const {
        return memoryProperties;
    }

    const std::vector<vk::QueueFamilyProperties> &PhysicalDeviceCandidate::getQueueFamilyProperties() const {
        return queueFamilyProperties;
    }

    const std::string PhysicalDeviceCandidate::getName() const {
        return std::string(deviceProperties.deviceName);
    }

    const SuccessStatus &PhysicalDeviceCandidate::getSuitableness() const {
        return suitableness;
    }

    const SwapchainSupportDetails &PhysicalDeviceCandidate::getSwapchainSupportDetails() const {
        return swapchainSupportDetails;
    }

    SwapchainSupportDetails::SwapchainSupportDetails(const vk::PhysicalDevice &device, const vk::SurfaceKHR &surface)
            : surfaceCapabilities(), surfaceFormats(), presentModes() {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
                (VkPhysicalDevice) device, (VkSurfaceKHR) surface,
                reinterpret_cast<VkSurfaceCapabilitiesKHR *>(&surfaceCapabilities)
        );
        surfaceFormats = device.getSurfaceFormatsKHR(surface);
        presentModes = device.getSurfacePresentModesKHR(surface);
    }

    const vk::SurfaceCapabilitiesKHR &SwapchainSupportDetails::getSurfaceCapabilities() const {
        return surfaceCapabilities;
    }

    const std::vector<vk::SurfaceFormatKHR> &SwapchainSupportDetails::getSurfaceFormats() const {
        return surfaceFormats;
    }

    const std::vector<vk::PresentModeKHR> &SwapchainSupportDetails::getPresentModes() const {
        return presentModes;
    }

}
#include <overeditor/graphics/device_context.h>

namespace overeditor::graphics {

    DeviceContext::DeviceContext(
            const PhysicalDeviceCandidate &dev,
            const Requirements &requirements,
            const vk::SurfaceKHR &surface
    ) : candidate(dev) {
        const graphics::QueueFamilyIndices &qIndices = dev.getIndices();

        std::vector<vk::DeviceQueueCreateInfo> createQueueInfos;
        uint32_t graphicsIndex, presentationIndex;
        if (!qIndices.getGraphics().tryGet(&graphicsIndex)) {
            throw std::runtime_error("Couldn't find graphics queue");
        }
        if (!qIndices.getPresentation().tryGet(&presentationIndex)) {
            throw std::runtime_error("Couldn't find presentation index");
        }
        const float queuePriority = 1.0f;
        createQueueInfos.emplace_back((vk::DeviceQueueCreateFlags) 0, graphicsIndex, 1, &queuePriority);
        if (graphicsIndex != presentationIndex) {
            createQueueInfos.emplace_back((vk::DeviceQueueCreateFlags) 0, presentationIndex, 1, &queuePriority);
        }
        LOG_INFO << "Creating " << createQueueInfos.size() << " queues: ";
        for (int i = 0; i < createQueueInfos.size(); ++i) {
            LOG_INFO << INDENTATION(1) << "Queue #" << i << ":";
            auto &q = createQueueInfos[i];
            LOG_INFO << INDENTATION(2) << "Count: " << q.queueCount;
            LOG_INFO << INDENTATION(2) << "Queue Family Index: " << q.queueFamilyIndex;
        }
        try {
            const auto &deviceExtensions = requirements.getRequiredExtensions();
            const auto &deviceLayers = requirements.getRequiredLayers();

            auto f = vk::DeviceCreateInfo(
                    (vk::DeviceCreateFlags) 0,
                    createQueueInfos.size(), createQueueInfos.data(),
                    deviceLayers.size(), deviceLayers.data(),
                    deviceExtensions.size(), deviceExtensions.data()
            );
            device = dev.getDevice().createDevice(f);
        } catch (std::exception &e) {
            LOG_FATAL << "Error while creating logical device: " << e.what();
            throw e;
        }
        queueContext = new QueueContext(device, qIndices, graphicsIndex, presentationIndex);
        const overeditor::graphics::SwapchainSupportDetails &scSupport = dev.getSwapchainSupportDetails();
        LOG_VECTOR_WITH("Surface formats", scSupport.getSurfaceFormats(), 1,
                        "Format: " << vk::to_string(value.format) << ", color space: "
                                   << vk::to_string(value.colorSpace));
        LOG_VECTOR_WITH("Presentation modes", scSupport.getPresentModes(), 1, vk::to_string(value));
        swapChainContext = new SwapChainContext(device, qIndices, scSupport, surface);
    }

    const PhysicalDeviceCandidate &DeviceContext::getCandidate() const {
        return candidate;
    }

    DeviceContext::~DeviceContext() {
        delete swapChainContext;
        delete queueContext;
        device.destroy();
    }

    QueueContext *DeviceContext::getQueueContext() const {
        return queueContext;
    }

    SwapChainContext *DeviceContext::getSwapChainContext() const {
        return swapChainContext;
    }

    const vk::Device &DeviceContext::getDevice() const {
        return device;
    }
}
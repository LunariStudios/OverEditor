#include <overeditor/application.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/graphics/queue_families.h>

#include <vulkan/vulkan.hpp>
#include <iostream>

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>


#define QUEUE_FAMILY_BIT_LOG(bit, name, flags) INDENTATION(3) << "* " << name << ": " << ((flags & bit) == (vk::QueueFlags) bit ? "present" : "absent")
namespace overeditor {
    Application::Application() {
        static plog::ColorConsoleAppender<plog::TxtFormatterUtcTime> consoleAppender;
        plog::init(plog::debug, &consoleAppender);
        auto appInfo = vk::ApplicationInfo(
                OVEREDITOR_NAME, // Application name
                OVEREDITOR_VERSION, // Application Version
                OVEREDITOR_NAME, // Engine name
                OVEREDITOR_VERSION, // Engine version
                VK_VERSION_1_1 // Vulkan version
        );
        auto createInfo = vk::InstanceCreateInfo((vk::InstanceCreateFlags) 0, &appInfo);
        instance = vk::createInstance(createInfo);
        uint32_t totalDevices;

        vkEnumeratePhysicalDevices(instance, &totalDevices, nullptr);

        if (totalDevices <= 0) {
            throw std::runtime_error("There are no Vulkan capable devices available!");
        }
        LOG_INFO << "Total available devices (" << totalDevices << "): ";
        std::vector<VkPhysicalDevice> devices(totalDevices);
        vkEnumeratePhysicalDevices(instance, &totalDevices, devices.data());
        uint32_t best = 0;
        size_t largestMemory = 0;
        std::string deviceName;
        for (int i = 0; i < devices.size(); ++i) {
            auto dev = vk::PhysicalDevice(devices[i]);
            VkPhysicalDeviceMemoryProperties m = dev.getMemoryProperties();
            VkPhysicalDeviceProperties deviceProperties = dev.getProperties();
            auto name = deviceProperties.deviceName;
            LOG_INFO << "Device # " << i << ": " << name << " (API: "
                     << deviceProperties.apiVersion << ")";
            LOG_INFO << INDENTATION(1) << " Memory Heaps (" << m.memoryHeapCount << "):";
            size_t totalMemory = 0;
            for (int j = 0; j < m.memoryHeapCount; ++j) {
                const VkMemoryHeap &heap = m.memoryHeaps[j];
                size_t mem = heap.size;
                totalMemory += mem;
                LOG_INFO << INDENTATION(2) << " Heap #" << j << ": " << FORMAT_BYTES_AS_GIB(heap.size);
            }
            LOG_INFO << INDENTATION(1) << "Total memory: " << FORMAT_BYTES_AS_GIB(totalMemory);
            if (totalMemory > largestMemory) {
                best = static_cast<uint32_t>(i);
                largestMemory = totalMemory;
                deviceName = name;
            }
        }
        auto physicalDevice = vk::PhysicalDevice(devices[best]);
        LOG_INFO << "Using device #" << best << " (" << deviceName << ")";
        auto properties = physicalDevice.getQueueFamilyProperties();
        LOG_INFO << "Device queue families (" << properties.size() << ")";
        overeditor::graphics::QueueFamilyIndices indices;
        for (uint32_t i = 0; i < properties.size(); ++i) {
            vk::QueueFamilyProperties &prop = properties[i];
            LOG_INFO << INDENTATION(1) << "Queue family #" << i << ":";
            LOG_INFO << INDENTATION(2) << "Flags: " << (uint32_t) prop.queueFlags;
            auto a = prop.queueFlags & vk::QueueFlagBits::eCompute;
            LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eCompute, "Compute", prop.queueFlags);
            LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eGraphics, "Graphics", prop.queueFlags);
            LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eTransfer, "Transfer", prop.queueFlags);
            LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eProtected, "Protected", prop.queueFlags);
            LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eSparseBinding, "Sparse binding", prop.queueFlags);
            LOG_INFO << INDENTATION(2) << "Count: " << prop.queueCount;
            indices.offer(i, prop);
        }
        LOG_INFO << "Family indices are:";
        const graphics::QueueFamily &graphics = indices.getGraphics();
        LOG_INFO << INDENTATION(1) << "Graphics: " << graphics;
        std::vector<vk::DeviceQueueCreateInfo> createQueueInfos;
        uint32_t graphicsIndex;
        if (!graphics.tryGet(&graphicsIndex)) {
            throw std::runtime_error("Couldn't find graphics queue");
        }
        createQueueInfos.emplace_back((vk::DeviceQueueCreateFlags) 0, graphicsIndex, 1);
        try {
            auto f = vk::DeviceCreateInfo(
                    (vk::DeviceCreateFlags) 0,
                    createQueueInfos.size(), createQueueInfos.data()
            );
            device = physicalDevice.createDevice(f);
        } catch (std::exception &e) {
            LOG_FATAL << "Error while creating logical device: " << e.what();
            throw e;
        }
        graphicsQueue = device.getQueue(graphicsIndex, 0);
    }

    Application::~Application() {
        instance.destroy();
        device.destroy();

    }

    void Application::run() {
        while (running) {
            //TODO: Variable delta time
            const float deltaTime = 1.0F / 60;
            sceneTick(deltaTime);
        }
    }
}
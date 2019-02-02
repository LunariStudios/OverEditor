#include <overeditor/application.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/memory_utility.h>
#include <vulkan/vulkan.hpp>
#include <iostream>

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

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
        device = physicalDevice.createDevice(vk::DeviceCreateInfo());
    }

    Application::~Application() {
        vkDestroyInstance(instance, nullptr);

    }
}
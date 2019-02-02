#include <overeditor/application.h>
#include <vulkan/vulkan.hpp>
#include <iostream>

namespace overeditor {
    Application::Application() {
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
        std::cout << "Total available devices: " << totalDevices << std::endl;
        std::vector<VkPhysicalDevice> devices(totalDevices);
        vkEnumeratePhysicalDevices(instance, &totalDevices, devices.data());
        for (const VkPhysicalDevice &device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            std::cout << "Device name: " << deviceProperties.deviceName << " (" << deviceProperties.deviceID
                      << ", Vendor: " << deviceProperties.vendorID << ", API: " << deviceProperties.apiVersion <<
                      ")";
        }

    }

    Application::~Application() {
        vkDestroyInstance(instance, nullptr);
    }
}
#include <overeditor/application.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/graphics/querying.h>
#include <vulkan/vulkan.hpp>

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <algorithm>

namespace overeditor {
    Application::Application() {
        static plog::ColorConsoleAppender<plog::TxtFormatterUtcTime> consoleAppender;
        plog::init(plog::debug, &consoleAppender);
        LOG_INFO << "Initializing GLFW";
        glfwInit();
        LOG_INFO << "Using GLFW: " << glfwGetVersionString();
        LOG_INFO << "Initializing extensions ";
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        LOG_INFO << "GLFW extensions are as follow (" << glfwExtensionCount << "):";
        for (int l = 0; l < glfwExtensionCount; ++l) {
            LOG_INFO << INDENTATION(1) << "* " << glfwExtensions[l];
        }
        auto appInfo = vk::ApplicationInfo(
                OVEREDITOR_NAME, // Application name
                OVEREDITOR_VERSION, // Application Version
                OVEREDITOR_NAME, // Engine name
                OVEREDITOR_VERSION, // Engine version
                VK_VERSION_1_1 // Vulkan version
        );

        auto createInfo = vk::InstanceCreateInfo((vk::InstanceCreateFlags) 0, &appInfo, 0, nullptr, glfwExtensionCount,
                                                 glfwExtensions);
        instance = vk::createInstance(createInfo);
        uint32_t totalDevices;

        vkEnumeratePhysicalDevices(instance, &totalDevices, nullptr);

        if (totalDevices <= 0) {
            throw std::runtime_error("There are no Vulkan capable devices available!");
        }
        LOG_INFO << totalDevices << " total available devices";
        // Query for best device
        std::vector<VkPhysicalDevice> devices(totalDevices);
        vkEnumeratePhysicalDevices(instance, &totalDevices, devices.data());
        std::vector<graphics::PhysicalDeviceCandidate> candidates;

        window = glfwCreateWindow(600, 800, OVEREDITOR_NAME, nullptr, nullptr);
        VkSurfaceKHR surface;
        LOG_INFO << "Surface b4: " << surface;
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
        LOG_INFO << "Surface after: " << surface;

        std::transform(devices.begin(), devices.end(), std::back_inserter(candidates),
                       [&](const VkPhysicalDevice &dev) {
                           return graphics::PhysicalDeviceCandidate(dev, surface);
                       }
        );
        LOG_INFO << "Total candidates " << candidates.size();
        for (int i = 0; i < candidates.size(); ++i) {
            const graphics::PhysicalDeviceCandidate &c = candidates[i];
            LOG_INFO << "Device #" << i << " (" << c.getName() << ")";
            const vk::PhysicalDeviceProperties &deviceProperties = c.getDeviceProperties();
            LOG_INFO << "Properties 2 reside @ " << &deviceProperties;
            const vk::PhysicalDeviceMemoryProperties &memoryProperties = c.getMemoryProperties();
            const std::vector<vk::QueueFamilyProperties> &queueFamilyProperties = c.getQueueFamilyProperties();
            LOG_INFO << INDENTATION(1) << " Memory Heaps (" << memoryProperties.memoryHeapCount << "):";
            size_t totalMemory = 0;
            for (int j = 0; j < memoryProperties.memoryHeapCount; ++j) {
                const VkMemoryHeap &heap = memoryProperties.memoryHeaps[j];
                bool isLocal = (heap.flags & (uint32_t) vk::MemoryHeapFlagBits::eDeviceLocal) ==
                               (uint32_t) vk::MemoryHeapFlagBits::eDeviceLocal;
                LOG_INFO << INDENTATION(2) << " Heap #" << j << ": " << FORMAT_BYTES_AS_GIB(heap.size) << ' '
                         << (isLocal ? "Local" : "Not local (invalid)");
                if (!isLocal) {
                    continue;
                }
                size_t mem = heap.size;
                totalMemory += mem;
            }
            LOG_INFO << INDENTATION(1) << "Total memory: " << FORMAT_BYTES_AS_GIB(totalMemory);
            LOG_INFO << "Device queue families (" << queueFamilyProperties.size() << ")";
            for (uint32_t k = 0; k < queueFamilyProperties.size(); ++k) {
                const vk::QueueFamilyProperties &prop = queueFamilyProperties[k];
                LOG_INFO << INDENTATION(1) << "Queue family #" << k << " (" << prop.queueCount << "):";
                LOG_INFO << INDENTATION(2) << "Flags: " << (uint32_t) prop.queueFlags;
                auto a = prop.queueFlags & vk::QueueFlagBits::eCompute;
                LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eCompute, "Compute", prop.queueFlags);
                LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eGraphics, "Graphics", prop.queueFlags);
                LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eTransfer, "Transfer", prop.queueFlags);
                LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eProtected, "Protected", prop.queueFlags);
                LOG_INFO << QUEUE_FAMILY_BIT_LOG(vk::QueueFlagBits::eSparseBinding, "Sparse binding", prop.queueFlags);
            }
        }
        // Remove devices that are not suitable
        candidates.erase(
                std::remove_if(candidates.begin(), candidates.end(),
                               [&](graphics::PhysicalDeviceCandidate &candidate) {
                                   return !candidate.isSuitable();
                               }
                ), candidates.end()
        );
        // Sort by highest score
        std::sort(candidates.begin(), candidates.end(),
                  [](graphics::PhysicalDeviceCandidate &a, graphics::PhysicalDeviceCandidate &b) {
                      return a.getScore() > b.getScore();
                  }
        );
        if (candidates.empty()) {
            LOG_FATAL << "There are no suitable devices!";
            return;
        }
        graphics::PhysicalDeviceCandidate elected = candidates[0];
        const graphics::QueueFamily &graphics = elected.getIndices().getGraphics();
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
            device = elected.getDevice().createDevice(f);
        } catch (std::exception &e) {
            LOG_FATAL << "Error while creating logical device: " << e.what();
            throw e;
        }
        graphicsQueue = device.getQueue(graphicsIndex, 0);

        static utility::Event<float>::EventListener quitter = [&](float dt) {
            running = !glfwWindowShouldClose(window);
            if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
                running = false;
            }
        };
        sceneTick.getEarlyStep() += &quitter;
        glfwShowWindow(window);
    }

    Application::~Application() {
        instance.destroy();
        //device.destroy();
        sceneTick.clear();

    }

    void Application::run() {
        while (running) {
            glfwPollEvents();
            //TODO: Variable delta time
            const float deltaTime = 1.0F / 60;
            sceneTick(deltaTime);
        }
    }
}
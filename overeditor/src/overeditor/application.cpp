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
    Application::Application() : instance(), device(), graphicsQueue(), running(), sceneTick(), window() {
        static plog::ColorConsoleAppender<plog::TxtFormatterUtcTime> consoleAppender;
        plog::init(plog::debug, &consoleAppender);
        LOG_INFO << "Initializing GLFW";
        glfwInit();
        glfwSetErrorCallback([](int i, const char *c) {
            LOG_ERROR << "GLFW Error (" << i << "): " << c;
        });

        LOG_INFO << "Using GLFW: " << glfwGetVersionString();
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char *> extensions;
        extensions.reserve(glfwExtensionCount);
        for (int l = 0; l < glfwExtensionCount; ++l) {
            extensions.push_back(glfwExtensions[l]);
        }
        LOG_INFO << "Vulkan extensions are as follow (" << extensions.size() << "):";
        for (auto extension : extensions) {
            LOG_INFO << INDENTATION(1) << "* " << extension;
        }
        auto appInfo = vk::ApplicationInfo(
                OVEREDITOR_NAME, // Application name
                OVEREDITOR_VERSION, // Application Version
                OVEREDITOR_NAME, // Engine name
                OVEREDITOR_VERSION, // Engine version
                VK_API_VERSION_1_1 // Vulkan version
        );

        std::vector<const char *> availableLayers = {"VK_LAYER_LUNARG_standard_validation"};
        auto createInfo = vk::InstanceCreateInfo((vk::InstanceCreateFlags) 0, &appInfo,
                                                 availableLayers.size(), availableLayers.data(),
                                                 extensions.size(), extensions.data());
        instance = vk::createInstance(createInfo);
        uint32_t totalDevices;

        // Check how many devices there are
        vkEnumeratePhysicalDevices(instance, &totalDevices, nullptr);
        if (totalDevices <= 0) {
            throw std::runtime_error("There are no Vulkan capable devices available!");
        }
        std::vector<vk::PhysicalDevice> devices(totalDevices);
        vkEnumeratePhysicalDevices(instance, &totalDevices, reinterpret_cast<VkPhysicalDevice *>(devices.data()));
        for (auto &dev : devices) {
            auto prop = dev.getProperties();
            LOG_INFO << INDENTATION(1) << "* " << prop.deviceName;
        }
        // Create Window and Surface
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(600, 800, OVEREDITOR_NAME, nullptr, nullptr);
        vk::SurfaceKHR surface;

        glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
        // Convert devices to candidates
        std::vector<graphics::PhysicalDeviceCandidate> candidates;
        candidates.reserve(devices.size());
        for (const vk::PhysicalDevice &device : devices) {
            candidates.emplace_back(device, surface);
        }
        LOG_INFO << "Device candidates (" << candidates.size() << "):";
        for (int i = 0; i < candidates.size(); ++i) {
            const graphics::PhysicalDeviceCandidate &c = candidates[i];
            LOG_INFO << INDENTATION(1) << "Device #" << i << " (\"" << c.getName() << "\") @ " << &c;
            const vk::PhysicalDeviceProperties &deviceProperties = c.getDeviceProperties();
            const vk::PhysicalDeviceMemoryProperties &memoryProperties = c.getMemoryProperties();
            const std::vector<vk::QueueFamilyProperties> &queueFamilyProperties = c.getQueueFamilyProperties();
            LOG_INFO << INDENTATION(2) << "Memory Heaps (" << memoryProperties.memoryHeapCount << "):";
            size_t totalMemory = 0;
            for (int j = 0; j < memoryProperties.memoryHeapCount; ++j) {
                const VkMemoryHeap &heap = memoryProperties.memoryHeaps[j];
                bool isLocal = (heap.flags & (uint32_t) vk::MemoryHeapFlagBits::eDeviceLocal) ==
                               (uint32_t) vk::MemoryHeapFlagBits::eDeviceLocal;
                LOG_INFO << INDENTATION(3) << " Heap #" << j << ": " << FORMAT_BYTES_AS_GIB(heap.size) << ' '
                         << (isLocal ? "Local" : "Not local (invalid)");
                if (!isLocal) {
                    continue;
                }
                size_t mem = heap.size;
                totalMemory += mem;
            }
            LOG_INFO << INDENTATION(2) << "Total memory: " << FORMAT_BYTES_AS_GIB(totalMemory);
            LOG_INFO << EMPTY_STRING;
            LOG_INFO << INDENTATION(2) << "Device queue families (" << queueFamilyProperties.size() << "):";
            for (uint32_t k = 0; k < queueFamilyProperties.size(); ++k) {
                const vk::QueueFamilyProperties &prop = queueFamilyProperties[k];
                LOG_INFO << INDENTATION(2) << "Queue family #" << k << " (" << prop.queueCount << "):";
                LOG_INFO << INDENTATION(3) << "Flags (" << (uint32_t) prop.queueFlags << "):";
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
        LOG_INFO << "Elected device is \"" << elected.getName() << "\"";
        const graphics::QueueFamily &graphics = elected.getIndices().getGraphics();
        std::vector<vk::DeviceQueueCreateInfo> createQueueInfos;
        uint32_t graphicsIndex;
        if (!graphics.tryGet(&graphicsIndex)) {
            throw std::runtime_error("Couldn't find graphics queue");
        }
        const float queuePriority = 1.0f;
        createQueueInfos.emplace_back((vk::DeviceQueueCreateFlags) 0, graphicsIndex, 1, &queuePriority);
        LOG_INFO << "Creating " << createQueueInfos.size() << " queues: ";
        for (int i = 0; i < createQueueInfos.size(); ++i) {
            LOG_INFO << INDENTATION(1) << "Queue #" << i << ":";
            auto &q = createQueueInfos[i];
            LOG_INFO << INDENTATION(2) << "Count: " << q.queueCount;
            LOG_INFO << INDENTATION(2) << "Queue Family Index: " << q.queueFamilyIndex;
        }
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
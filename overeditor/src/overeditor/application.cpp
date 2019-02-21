#include <overeditor/application.h>
#include <overeditor/overeditor_constants.h>
#include <overeditor/utility/string_utility.h>
#include <overeditor/utility/memory_utility.h>
#include <overeditor/graphics/queue_families.h>
#include <overeditor/graphics/querying.h>
#include <overeditor/graphics/requirements.h>
#include <vulkan/vulkan.hpp>

#include <plog/Log.h>
#include <plog/Appenders/ColorConsoleAppender.h>

#include <algorithm>

namespace overeditor {
    void onError(int code, const char *msg) {
        LOG_ERROR << "GLFW Error (" << code << "): " << msg;
    }


#define LOG_REQUIREMENTS(name, v) LOG_INFO << "Required " << name << " are (" << v.size() << "):";\
    LOG_VECTOR_NO_HEADER(v, 1);

    Application::Application()
            : instance(), device(), surface(), swapchain(), graphicsQueue(), presentationQueue(), running(),
              sceneTick(), window(), instanceSuitable() {
        static plog::ColorConsoleAppender<plog::TxtFormatterUtcTime> consoleAppender;
        plog::init(plog::debug, &consoleAppender);
        glfwInit();
        glfwSetErrorCallback(onError);
        LOG_INFO << "Using GLFW: " << glfwGetVersionString();
        auto requirements = overeditor::graphics::VulkanRequirements::createOverEditorRequirements();
        const auto &instanceRequirements = requirements.getInstanceRequirements();
        const std::vector<const char *> &instanceRequiredExtensions = instanceRequirements.getRequiredExtensions();
        const auto &instanceRequiredLayers = instanceRequirements.getRequiredLayers();

        // Get available instance extensions and layers
        uint32_t instanceLayerCount, instanceExtensionCount;
        // Find instance extensions
        vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
        std::vector<vk::ExtensionProperties> availableInstanceExtensions(instanceExtensionCount);
        vkEnumerateInstanceExtensionProperties(
                nullptr, &instanceExtensionCount,
                reinterpret_cast<VkExtensionProperties *>(availableInstanceExtensions.data())
        );
        // Find instance layers
        vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
        std::vector<vk::LayerProperties> availableInstanceLayers(instanceLayerCount);
        vkEnumerateInstanceLayerProperties(
                &instanceLayerCount,
                reinterpret_cast<VkLayerProperties *>(availableInstanceLayers.data())
        );
        LOG_REQUIREMENTS("instance extensions", instanceRequiredExtensions);
        LOG_VECTOR_WITH("Available instance extensions (" << availableInstanceExtensions.size() << "):",
                        availableInstanceExtensions, 1, value.extensionName);
        LOG_REQUIREMENTS("instance layers", instanceRequiredLayers);
        LOG_VECTOR_WITH("Available instance layers (" << availableInstanceLayers.size() << "):",
                        availableInstanceLayers, 1, value.layerName);
        instanceRequirements.checkRequirements(availableInstanceExtensions, availableInstanceLayers, instanceSuitable);
        if (instanceSuitable.isSuccessful()) {
            LOG_INFO << "Successfully found all required extensions and layers";
        } else {
            LOG_ERROR << "Error(s) occoured while finding instance extensions and layers:";
            instanceSuitable.printErrors();
        }
        const auto &deviceRequirements = requirements.getDeviceRequirements();
        const auto &deviceExtensions = deviceRequirements.getRequiredExtensions();
        const auto &deviceLayers = deviceRequirements.getRequiredLayers();

        auto appInfo = vk::ApplicationInfo(
                OVEREDITOR_NAME, // Application name
                OVEREDITOR_VERSION, // Application Version
                OVEREDITOR_NAME, // Engine name
                OVEREDITOR_VERSION, // Engine version
                VK_API_VERSION_1_1 // Vulkan version
        );


        auto createInfo = vk::InstanceCreateInfo(
                (vk::InstanceCreateFlags) 0, &appInfo,
                instanceRequiredLayers.size(), instanceRequiredLayers.data(),
                instanceRequiredExtensions.size(), instanceRequiredExtensions.data()
        );
        instance = vk::createInstance(createInfo);
        uint32_t totalDevices;

        // Check how many devices there are
        vkEnumeratePhysicalDevices(instance, &totalDevices, nullptr);
        if (totalDevices <= 0) {
            throw std::runtime_error("There are no Vulkan capable devices available!");
        }
        std::vector<vk::PhysicalDevice> devices(totalDevices);
        vkEnumeratePhysicalDevices(instance, &totalDevices, reinterpret_cast<VkPhysicalDevice *>(devices.data()));
        /*swapchain = device.createSwapchainKHR(
                vk::SwapchainCreateInfoKHR((vk::SwapchainCreateFlagsKHR) 0, surface,)
        );*/
        // Create Window and Surface
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(600, 800, OVEREDITOR_NAME, nullptr, nullptr);
        glfwCreateWindowSurface(instance, window, nullptr, reinterpret_cast<VkSurfaceKHR *>(&surface));
        // Convert devices to candidates
        std::vector<graphics::PhysicalDeviceCandidate> candidates;
        candidates.reserve(devices.size());
        for (const vk::PhysicalDevice &device : devices) {
            candidates.emplace_back(deviceRequirements, device, surface);
        }
        LOG_INFO << "Device candidates (" << candidates.size() << "):";
        for (int i = 0; i < candidates.size(); ++i) {
            const graphics::PhysicalDeviceCandidate &c = candidates[i];
            LOG_INFO << INDENTATION(1) << "Device #" << i << " (\"" << c.getName() << "\", score: " << c.getScore()
                     << ")";
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
            LOG_INFO << INDENTATION(2) << "Device queue families (" << queueFamilyProperties.size() << "):";
            for (uint32_t k = 0; k < queueFamilyProperties.size(); ++k) {
                const vk::QueueFamilyProperties &prop = queueFamilyProperties[k];
                LOG_INFO << INDENTATION(2) << "Queue family #" << k << " (" << prop.queueCount << "):";
                LOG_INFO << INDENTATION(3) << "Flags (" << (uint32_t) prop.queueFlags << "):";
                auto a = prop.queueFlags & vk::QueueFlagBits::eCompute;
                LOG_INFO << LOG_QUEUE_FAMILY_BIT(vk::QueueFlagBits::eCompute, "Compute", prop.queueFlags);
                LOG_INFO << LOG_QUEUE_FAMILY_BIT(vk::QueueFlagBits::eGraphics, "Graphics", prop.queueFlags);
                LOG_INFO << LOG_QUEUE_FAMILY_BIT(vk::QueueFlagBits::eTransfer, "Transfer", prop.queueFlags);
                LOG_INFO << LOG_QUEUE_FAMILY_BIT(vk::QueueFlagBits::eProtected, "Protected", prop.queueFlags);
                LOG_INFO << LOG_QUEUE_FAMILY_BIT(vk::QueueFlagBits::eSparseBinding, "Sparse binding", prop.queueFlags);
            }
        }
        // Remove devices that are not suitable
        candidates.erase(
                std::remove_if(candidates.begin(), candidates.end(),
                               [&](graphics::PhysicalDeviceCandidate &candidate) {
                                   return !candidate.getSuitableness().isSuccessful();
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
        const graphics::QueueFamilyIndices &qIndices = elected.getIndices();

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
        createQueueInfos.emplace_back((vk::DeviceQueueCreateFlags) 0, presentationIndex, 1, &queuePriority);
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
        presentationQueue = device.getQueue(presentationIndex, 0);
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
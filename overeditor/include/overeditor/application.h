#ifndef OVEREDITOR_APPLICATION_H
#define OVEREDITOR_APPLICATION_H

#include <overeditor/utility/step_function.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

/**
 * The current version of OverEditor.
 * The OVEREDITOR_VERSION_MAJOR, OVEREDITOR_VERSION_MINOR and OVEREDITOR_VERSION_PATCH macros are defined by CMake
 */
#define OVEREDITOR_VERSION VK_MAKE_VERSION(OVEREDITOR_VERSION_MAJOR, OVEREDITOR_VERSION_MINOR, OVEREDITOR_VERSION_PATCH)
/**
 *
 */
#define OVEREDITOR_NAME "OverEditor"
namespace overeditor {

    class Application {
    private:
        vk::Instance instance;
        vk::Device device;
        vk::Queue graphicsQueue;
        bool running;
        utility::StepFunction<float> sceneTick;
        GLFWwindow * window;
    public:
        Application();

        virtual ~Application();

        void run();

    };
}

#endif
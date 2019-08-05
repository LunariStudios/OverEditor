#ifndef OVEREDITOR_APPLICATION_H
#define OVEREDITOR_APPLICATION_H

#include <overeditor/utility/step_function.h>
#include <overeditor/utility/success_status.h>
#include <overeditor/graphics/swapchain_context.h>
#include <overeditor/graphics/device_context.h>
#include <overeditor/ecs/systems/rendering.h>
#include <entityx/entityx.h>
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

    class Application : public entityx::EntityX {
    private:
        // Vulkan members
        vk::Instance instance;
        vk::SurfaceKHR surface;
        // Graphics layer members
        graphics::DeviceContext *deviceContext;
        // Engine layer members
        bool running;
        utility::StepFunction<float> sceneTick;
        utility::SuccessStatus instanceSuitable;
        GLFWwindow *window;
        std::shared_ptr<overeditor::systems::graphics::RenderingSystem> renderingSystem;
    public:
        Application();

        virtual ~Application();

        void run();

        graphics::DeviceContext *getDeviceContext() const;

         utility::StepFunction<float> &getSceneTick() ;

        const std::shared_ptr<systems::graphics::RenderingSystem> &getRenderingSystem() const;
    };
}

#endif
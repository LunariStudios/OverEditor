#ifndef OVEREDITOR_RENDERING_H
#define OVEREDITOR_RENDERING_H

#include <entityx/entityx.h>
#include <overeditor/ecs/components/common.h>

namespace overeditor::systems::graphics {
    class RenderingSystem : public entityx::System<RenderingSystem> {
    private:
        const overeditor::graphics::DeviceContext *context;
        vk::CommandBuffer primaryBuffer;
        vk::CommandPool pool;
        std::vector<vk::Framebuffer> framebuffers;
        vk::Semaphore imageAvailableSemaphore, renderFinishedSemaphore;

        void render(
                Camera &camera,
                const Transform &cameraTransform,
                entityx::EntityManager &entities
        );

    public:
        vk::RenderPass renderPass;

        explicit RenderingSystem(
                const overeditor::graphics::DeviceContext &context
        );

        void update(
                entityx::EntityManager &entities,
                entityx::EventManager &events,
                entityx::TimeDelta dt
        ) override;

    };

}

#endif

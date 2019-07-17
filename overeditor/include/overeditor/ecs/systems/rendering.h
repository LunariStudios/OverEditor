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
    public:
        vk::RenderPass renderPass;

        explicit RenderingSystem(
                const overeditor::graphics::DeviceContext &context
        ) {
            RenderingSystem::context = &context;
            auto scContext = context.getSwapChainContext();
            vk::AttachmentDescription colorAttachment(
                    (vk::AttachmentDescriptionFlags) 0, // Flags
                    scContext->getSwapchainFormat(), // Format
                    vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear,
                    vk::AttachmentStoreOp::eStore,
                    vk::AttachmentLoadOp::eDontCare,
                    vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined,
                    vk::ImageLayout::ePresentSrcKHR
            );
            vk::AttachmentReference colorAttachmentRef(
                    0,
                    vk::ImageLayout::eColorAttachmentOptimal
            );
            vk::SubpassDescription subpass(
                    (vk::SubpassDescriptionFlags) 0,
                    vk::PipelineBindPoint::eGraphics,
                    0, nullptr,
                    1, &colorAttachmentRef
            );
            auto dep = vk::SubpassDependency(
                    VK_SUBPASS_EXTERNAL, 0,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    (vk::AccessFlags) 0,
                    vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
            );
            auto &device = context.getDevice();
            pool = device.createCommandPool(
                    vk::CommandPoolCreateInfo(
                            (vk::CommandPoolCreateFlags) vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
                            context.getQueueContext()->getFamilyIndices().getGraphics().get()
                    )
            );
            renderPass = device.createRenderPass(
                    vk::RenderPassCreateInfo(
                            (vk::RenderPassCreateFlags) 0,
                            1, &colorAttachment,
                            1, &subpass,
                            1, &dep
                    )
            );
            primaryBuffer = device.allocateCommandBuffers(
                    vk::CommandBufferAllocateInfo(
                            pool,
                            vk::CommandBufferLevel::ePrimary,
                            1
                    )
            )[0];
            auto &imgs = scContext->getSwapchainImages();
            size_t count = imgs.size();
            framebuffers.reserve(count);
            auto ex = scContext->getSwapchainExtent();
            for (size_t i = 0; i < count; i++) {
                auto &view = imgs[i].getView();
                framebuffers.emplace_back(
                        device.createFramebuffer(
                                vk::FramebufferCreateInfo(
                                        (vk::FramebufferCreateFlags) 0,
                                        renderPass,
                                        1, &view,
                                        ex.width, ex.height, 1
                                )
                        )
                );
            }
            imageAvailableSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
            renderFinishedSemaphore = device.createSemaphore(vk::SemaphoreCreateInfo());
        }

        void update(
                entityx::EntityManager &entities,
                entityx::EventManager &events,
                entityx::TimeDelta dt
        ) override {
            std::vector<vk::CommandBuffer> secondaryBuffers;

            uint32_t imageIndex;
            const auto &device = context->getDevice();
            const auto &swapchain = context->getSwapChainContext()->getSwapchain();
            device.acquireNextImageKHR(
                    swapchain,
                    std::numeric_limits<uint64_t>::max(),
                    imageAvailableSemaphore,
                    nullptr,
                    &imageIndex
            );

            vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
            //Re-record buffer
            entityx::ComponentHandle<Transform> transform;
            entityx::ComponentHandle<Drawable> drawable;
            for (entityx::Entity e : entities.entities_with_components(transform, drawable)) {
                auto t = transform.get();
                auto d = drawable.get();
                secondaryBuffers.emplace_back(d->buf);
            }
            if (secondaryBuffers.empty()) {
                //Nothing to draw
                return;
            }
            primaryBuffer.begin(
                    vk::CommandBufferBeginInfo(
                            (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse
                    )
            );

            vk::ClearValue value = vk::ClearColorValue((std::array<float, 4>) {
                    0.0F, 0.0F, 0.0F, 1.0F
            });
            primaryBuffer.beginRenderPass(
                    vk::RenderPassBeginInfo(
                            renderPass,
                            framebuffers[imageIndex],
                            vk::Rect2D(vk::Offset2D(),
                                       context->getSwapChainContext()->getSwapchainExtent()),
                            1,
                            &value
                    ),
                    vk::SubpassContents::eSecondaryCommandBuffers
            );
            primaryBuffer.executeCommands(secondaryBuffers);
            primaryBuffer.endRenderPass();
            primaryBuffer.end();
            // Submit
            vk::SubmitInfo info = vk::SubmitInfo(
                    1, &imageAvailableSemaphore, waitStages,
                    1, &primaryBuffer,
                    1, &renderFinishedSemaphore
            );
            auto &queue = context->getQueueContext()->getGraphicsQueue();
            vkAssertOk(
                    queue.submit(1, &info, nullptr)
            )
            vkAssertOk(
                    queue.presentKHR(
                            vk::PresentInfoKHR(
                                    1, &renderFinishedSemaphore,
                                    1, &swapchain,
                                    &imageIndex
                            )
                    )
            )
            queue.waitIdle();
        }
    };
}

#endif

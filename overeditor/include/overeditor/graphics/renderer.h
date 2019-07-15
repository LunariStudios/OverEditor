#ifndef OVEREDITOR_RENDERER_H
#define OVEREDITOR_RENDERER_H

#include <vulkan/vulkan.hpp>
#include <overeditor/utility/vulkan_utility.h>

class Renderer {
private:
    vk::Semaphore imageAvailableSemaphore, renderFinishedSemaphore;
    const vk::Device *device;
public:
    explicit Renderer(const vk::Device &device) {
        Renderer::device = &device;
        imageAvailableSemaphore = device.createSemaphore(
                vk::SemaphoreCreateInfo()
        );
        renderFinishedSemaphore = device.createSemaphore(
                vk::SemaphoreCreateInfo()
        );
    }

    ~Renderer() {
        device->destroy(imageAvailableSemaphore);
        device->destroy(renderFinishedSemaphore);
    }

    void render(
            const vk::Queue &queue,
            const vk::SwapchainKHR &swapchain,
            const overeditor::graphics::shaders::GraphicsPipeline &pipeline
    ) {
        uint32_t imageIndex;
        device->acquireNextImageKHR(swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, nullptr,
                                    &imageIndex);

        vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::SubmitInfo info = vk::SubmitInfo(
                1, &imageAvailableSemaphore, waitStages,
                1, &(pipeline.getCommandBuffers()[imageIndex]),
                1, &renderFinishedSemaphore
        );
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

#endif

#ifndef OVEREDITOR_COMMON_H
#define OVEREDITOR_COMMON_H

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/buffers/vertices.h>

struct Transform {
    explicit Transform(
            const glm::vec3 &position = glm::vec3(),
            const glm::quat &rotation = glm::quat(1, 0, 0, 0),
            const glm::vec3 &scale = glm::vec3()
    ) : position(position),
        rotation(rotation),
        scale(scale) {}

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct Drawable {
public:
    vk::CommandBuffer buf;

    static Drawable forGeometry(
            const overeditor::graphics::DeviceContext &deviceContext,
            const vk::Pipeline &pipeline,
            const vk::CommandPool &pool,
            const vk::RenderPass &renderPass,
            uint32_t subpass,
            const overeditor::graphics::GeometryBuffer &buffer
    ) {
        auto &device = deviceContext.getDevice();
        vk::CommandBuffer buf;
        auto info = vk::CommandBufferAllocateInfo(
                pool,
                vk::CommandBufferLevel::eSecondary,
                1

        );
        device.allocateCommandBuffers(
                &info,
                &buf
        );

        auto inh = vk::CommandBufferInheritanceInfo(
                renderPass, subpass
        );
        buf.begin(
                vk::CommandBufferBeginInfo(
                        (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse |
                        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
                        &inh
                )
        );
        buf.bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                pipeline
        );
        buf.draw(3, 1, 0, 0);
        buf.end();
        return Drawable(
                buf
        );

    }

    explicit Drawable(const vk::CommandBuffer &buf = nullptr) : buf(buf) {

    }
};

#endif //OVEREDITOR_COMMON_H

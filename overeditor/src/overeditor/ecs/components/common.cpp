#include <utility>

#include <utility>

#include <overeditor/ecs/components/common.h>


glm::vec3 Transform::getForward(const Transform &transform) {
    return transform.rotation * glm::vec3(0, 0, 1);
}

glm::vec3 Transform::getUp(const Transform &transform) {
    return transform.rotation * glm::vec3(0, 1, 0);
}

glm::vec3 Transform::getRight(const Transform &transform) {
    return transform.rotation * glm::vec3(1, 0, 0);
}

Transform::Transform(
        const glm::vec3 &position,
        const glm::quat &rotation,
        const glm::vec3 &scale
) : position(position),
    rotation(rotation),
    scale(scale) {}

void append_sizes_to(
        const overeditor::utility::DescriptorLayout &layout,
        std::vector<vk::DescriptorPoolSize> &output
) {
    auto elements = layout.getElements();
    for (auto &element : elements) {
        if (element.isImported()) {
            continue;
        }
        output.emplace_back(
                vk::DescriptorType::eUniformBuffer,
                element.getElementCount()
        );
    }
}

std::vector<vk::DescriptorPoolSize> get_sizes(
        const overeditor::graphics::shaders::Shader &shader
) {
    std::vector<vk::DescriptorPoolSize> result;
    auto vertLayout = shader.getVertSource().getDescriptorLayouts();
    auto fragLayout = shader.getFragSource().getDescriptorLayouts();
    for (const auto &layout : vertLayout) {
        append_sizes_to(layout, result);

    }
    for (const auto &layout : fragLayout) {
        append_sizes_to(layout, result);

    }
    return result;
}

Drawable Drawable::forGeometry(
        const overeditor::graphics::DeviceContext &deviceContext,
        const overeditor::graphics::shaders::Shader &shader,
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
    LOG_INFO << "Begin recording";

    buf.begin(
            vk::CommandBufferBeginInfo(
                    (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse |
                    vk::CommandBufferUsageFlagBits::eRenderPassContinue,
                    &inh
            )
    );
    std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = get_sizes(shader);
    vk::DescriptorPool descriptorPool = nullptr;
    std::vector<vk::DescriptorSet> sets;
    if (!descriptorPoolSizes.empty()) {

        const auto &layouts = shader.getDescriptorsLayouts();
        descriptorPool = device.createDescriptorPool(
                vk::DescriptorPoolCreateInfo(
                        (vk::DescriptorPoolCreateFlags) 0,
                        layouts.size(),
                        descriptorPoolSizes.size(),
                        descriptorPoolSizes.data()
                )
        );
        sets = device.allocateDescriptorSets(
                vk::DescriptorSetAllocateInfo(
                        descriptorPool,
                        layouts.size(),
                        layouts.data()
                )
        );
        if (!sets.empty()) {
            buf.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    shader.getLayout(),
                    0,
                    sets,
                    {}
            );
        }
    }
    buf.bindPipeline(
            vk::PipelineBindPoint::eGraphics,
            shader.getPipeline()
    );
    buf.bindVertexBuffers(
            0,
            {
                    buffer.getVertexBuffer()
            }, {
                    0
            }
    );
    buf.bindIndexBuffer(buffer.getIndexBuffer(), 0, vk::IndexType::eUint16);
    buf.drawIndexed(
            buffer.getTotalIndices(), 1, 0, 0, 0
    );
    buf.end();
    LOG_INFO << "End recording";
    return Drawable(
            buf,
            descriptorPool,
            &shader,
            sets
    );

}

Drawable::Drawable(
        const vk::CommandBuffer &buf,
        const vk::DescriptorPool &pool,
        const overeditor::graphics::shaders::Shader *shader,
        std::vector<vk::DescriptorSet> descriptors
) : buf(buf), shader(shader),
    descriptorPool(pool), descriptors(std::move(descriptors)) {
}

Camera::Camera(
        float depth,
        float fieldOfView,
        float aspectRatio
) : depth(depth),
    fieldOfView(fieldOfView),
    aspectRatio(aspectRatio),
    matrices() {

}

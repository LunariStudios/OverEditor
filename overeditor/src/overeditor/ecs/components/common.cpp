#include <utility>
#include <overeditor/ecs/components/common.h>
#include <overeditor/utility/vulkan_utility.h>
#include <overeditor/graphics/common.h>

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

void append_buffers_to(
        const overeditor::ShaderSource &src,
        const overeditor::DeviceContext &dev,
        std::vector<vk::Buffer> &bufs,
        std::vector<vk::DeviceMemory> &memories
) {

    const auto &device = dev.getDevice();
    const auto &descriptors = src.getDescriptorLayouts();
    std::vector<uint32_t> q = {
            dev.getQueueContext()->getFamilyIndices().getGraphics().get()
    };
    for (const auto &d : descriptors) {
        for (const auto &e : d.getElements()) {
            if (e.isImported()) {
                continue;
            }
            vk::Buffer buf;
            vk::DeviceMemory mem;
            overeditor::createBuffer(dev, e.getSize(), buf, mem, vk::BufferUsageFlagBits::eUniformBuffer);
            bufs.emplace_back(buf);
            memories.emplace_back(mem);
        }
    }
}

void append_sizes_to(
        const overeditor::DescriptorLayout &layout,
        std::vector<vk::DescriptorPoolSize> &output
) {
    auto elements = layout.getElements();
    for (overeditor::DescriptorElement &element : elements) {
        output.emplace_back(
                element.getType(),
                element.getElementCount()
        );
    }
}

std::vector<vk::DescriptorPoolSize> get_sizes(
        const overeditor::Shader &shader
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
        const overeditor::DeviceContext &deviceContext,
        const overeditor::Shader &shader,
        const vk::CommandPool &pool,
        const vk::RenderPass &renderPass,
        uint32_t subpass,
        const overeditor::GeometryBuffer &buffer
) {
    auto &device = deviceContext.getDevice();
    auto controller = overeditor::DescriptorsController::createFor(
            deviceContext,
            shader.collectDescriptors(),
            shader.getDescriptorsLayouts()
    );

    return Drawable(
            &buffer,
            &shader,
            pool
    );

}

Drawable::Drawable(
        const overeditor::GeometryBuffer *geometry,
        const overeditor::Shader *shader,
        const vk::CommandPool &pool
) : geometry(geometry), shader(shader), pool(pool), model() {

}

DrawingInstructions DrawingInstructions::createFor(
        const overeditor::DeviceContext &dev,
        const Camera &camera,
        const Drawable &drawable
) {
    const vk::Device &device = dev.getDevice();
    DrawingInstructions i;
    const overeditor::Shader *shader = drawable.shader;
    i.descriptors = overeditor::DescriptorsController::createFor(
            dev,
            shader->collectDescriptors(),
            shader->getDescriptorsLayouts()
    );
    std::vector<overeditor::DescriptorLayout> layouts = {
            overeditor::DescriptorLayout(
                    {
                            //Model matrix
                            overeditor::DescriptorElement(
                                    sizeof(glm::mat4),
                                    1,
                                    vk::DescriptorType::eUniformBuffer
                            )
                    }
            )
    };
    const auto &descriptors = i.descriptors.getDescriptors();
    std::vector<vk::WriteDescriptorSet> writes;
    device.updateDescriptorSets(
            {
                    overeditor::updateModelMatrixWrite(i.descriptors, *shader)
            }, {
            }
    );
    i.drawCommand = Drawable::exportBufferFor(camera, i.descriptors, device, drawable.pool, 0, drawable);
    return i;
}

DrawingInstructions::DrawingInstructions(
        const overeditor::DescriptorsController &descriptors,
        const vk::CommandBuffer &drawCommand
) : descriptors(descriptors), drawCommand(drawCommand) {}

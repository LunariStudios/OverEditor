#ifndef OVEREDITOR_COMMON_H
#define OVEREDITOR_COMMON_H

#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/buffers/vertices.h>
#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/graphics/descriptors.h>

struct Transform {
    static glm::vec3 getForward(const Transform &transform);

    static glm::vec3 getUp(const Transform &transform);

    static glm::vec3 getRight(const Transform &transform);

    explicit Transform(
            const glm::vec3 &position = glm::vec3(0, 0, 0),
            const glm::quat &rotation = glm::quat(1, 0, 0, 0),
            const glm::vec3 &scale = glm::vec3(1, 1, 1)
    );

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

using ShaderPtr =  overeditor::Shader *;

std::vector<vk::DescriptorPoolSize> get_sizes(
        const overeditor::Shader &shader
);

struct CameraMatrices {
    glm::mat4 view;
    glm::mat4 project;
};
struct DrawingInstructions;


struct Camera {
    float depth;
    float fieldOfView;
    float aspectRatio;
    vk::RenderPass renderPass;
    CameraMatrices *matrices;
    vk::Buffer matricesBuffer;
    vk::DeviceMemory matricesMemory;
    std::map<ShaderPtr, DrawingInstructions> instructions;

    static Camera createFor(
            const overeditor::DeviceContext &dev,
            vk::RenderPass renderPass,
            float depth,
            float fieldOfView,
            float aspectRatio
    ) {
        auto &device = dev.getDevice();
        Camera camera{
                depth,
                fieldOfView,
                aspectRatio,
                renderPass

        };
        overeditor::createBuffer(
                dev, sizeof(CameraMatrices),
                camera.matricesBuffer,
                camera.matricesMemory,
                vk::BufferUsageFlagBits::eUniformBuffer
        );


        device.mapMemory(
                camera.matricesMemory,
                0,
                sizeof(CameraMatrices),
                (vk::MemoryMapFlags) 0,
                reinterpret_cast<void **>(&camera.matrices)
        );
        return camera;
    }
};

struct Drawable;

struct DrawingInstructions {
public:
    overeditor::DescriptorsController descriptors;
    vk::CommandBuffer drawCommand;

    DrawingInstructions() = default;


    DrawingInstructions(
            const overeditor::DescriptorsController &descriptors,
            const vk::CommandBuffer &drawCommand
    );

    static DrawingInstructions createFor(
            const overeditor::DeviceContext &dev,
            const Camera &camera,
            const Drawable &drawable
    );
};

struct Drawable {
public:
    const overeditor::GeometryBuffer *geometry;
    const overeditor::Shader *shader;
    const vk::CommandPool pool;

    static vk::CommandBuffer exportBufferFor(
            const Camera &camera,
            const overeditor::DescriptorsController &descriptorsController,
            const vk::Device &device,
            const vk::CommandPool &pool,
            uint32_t subpass,
            const Drawable &drawable
    ) {
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
                camera.renderPass, subpass
        );
        LOG_INFO << "Begin recording";

        buf.begin(
                vk::CommandBufferBeginInfo(
                        (vk::CommandBufferUsageFlags) vk::CommandBufferUsageFlagBits::eSimultaneousUse |
                        vk::CommandBufferUsageFlagBits::eRenderPassContinue,
                        &inh
                )
        );

        const auto &shaderRef = *drawable.shader;
        buf.bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                shaderRef.getPipeline()
        );
        auto &geometry = *drawable.geometry;
        auto descriptors = descriptorsController.getDescriptors();
        if (!descriptors.empty()) {
            buf.bindDescriptorSets(
                    vk::PipelineBindPoint::eGraphics,
                    shaderRef.getLayout(),
                    0,
                    descriptors,
                    {

                    }
            );
        }
        buf.bindVertexBuffers(
                0,
                {
                        geometry.getVertexBuffer()
                }, {
                        0
                }
        );
        buf.bindIndexBuffer(geometry.getIndexBuffer(), 0, vk::IndexType::eUint16);
        buf.drawIndexed(
                geometry.getTotalIndices(), 1, 0, 0, 0
        );
        buf.end();
        return buf;
    }

    static Drawable forGeometry(
            const overeditor::DeviceContext &deviceContext,
            const overeditor::Shader &shader,
            const vk::CommandPool &pool,
            const vk::RenderPass &renderPass,
            uint32_t subpass,
            const overeditor::GeometryBuffer &buffer
    );

    Drawable(
            const overeditor::GeometryBuffer *geometry,
            const overeditor::Shader *shader,
            const vk::CommandPool &pool
    );

};

#endif //OVEREDITOR_COMMON_H

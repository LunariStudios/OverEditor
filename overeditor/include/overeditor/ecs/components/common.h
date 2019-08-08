#ifndef OVEREDITOR_COMMON_H
#define OVEREDITOR_COMMON_H

#include <map>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/buffers/vertices.h>
#include <overeditor/graphics/shaders/shader.h>

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

using ShaderPtr = const overeditor::graphics::shaders::Shader *;

std::vector<vk::DescriptorPoolSize> get_sizes(
        const overeditor::graphics::shaders::Shader &shader
);

struct CameraMatrices {
    glm::mat4 view;
    glm::mat4 project;
};

struct Camera {
    float depth;
    float fieldOfView;
    float aspectRatio;
    vk::RenderPass renderPass;
    std::map<ShaderPtr, vk::CommandBuffer> shaderBinds;
    CameraMatrices matrices;

    Camera(
            float depth,
            float fieldOfView,
            float aspectRatio
    );
};

struct Drawable {
public:
    const vk::CommandBuffer buf;
    const overeditor::graphics::GeometryBuffer *geometry;
    const overeditor::graphics::shaders::Shader *shader;
    const vk::DescriptorPool descriptorPool;
    const std::vector<vk::DescriptorSet> descriptors;


    static vk::CommandBuffer exportBufferFor(
            const Camera &camera,
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
        std::vector<vk::DescriptorPoolSize> descriptorPoolSizes = get_sizes(shaderRef);
        vk::DescriptorPool descriptorPool = nullptr;
        std::vector<vk::DescriptorSet> sets;
        if (!descriptorPoolSizes.empty()) {

            const auto &layouts = shaderRef.getDescriptorsLayouts();
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
                        shaderRef.getLayout(),
                        0,
                        sets,
                        {}
                );
            }
        }
        buf.bindPipeline(
                vk::PipelineBindPoint::eGraphics,
                shaderRef.getPipeline()
        );
        auto &geometry = *drawable.geometry;
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
            const overeditor::graphics::DeviceContext &deviceContext,
            const overeditor::graphics::shaders::Shader &shader,
            const vk::CommandPool &pool,
            const vk::RenderPass &renderPass,
            uint32_t subpass,
            const overeditor::graphics::GeometryBuffer &buffer
    );

    explicit Drawable(
            const overeditor::graphics::GeometryBuffer *buffer = nullptr,
            const vk::CommandBuffer &buf = nullptr,
            const vk::DescriptorPool &descriptorPool = nullptr,
            const overeditor::graphics::shaders::Shader *shader = nullptr,
            std::vector<vk::DescriptorSet> descriptors = std::vector<vk::DescriptorSet>()
    );
};

#endif //OVEREDITOR_COMMON_H

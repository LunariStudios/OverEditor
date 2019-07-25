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
            const glm::vec3 &position = glm::vec3(),
            const glm::quat &rotation = glm::quat(1, 0, 0, 0),
            const glm::vec3 &scale = glm::vec3()
    );

    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
};

struct CameraMatrices {
    glm::mat4 view;
    glm::mat4 project;
};

struct Camera {
    using ShaderPtr = overeditor::graphics::shaders::Shader *;
    float depth;
    float fieldOfView;
    float aspectRatio;
    std::map<ShaderPtr, vk::CommandBuffer> shaderBinds;
    vk::Buffer matricesBuf;
    vk::DeviceMemory matricesMemory;
    vk::DescriptorSet matricesSet;
    vk::DescriptorSetLayout matricesLayout;
    vk::DescriptorPool descriptorPool;

    static Camera forDevice(
            const overeditor::graphics::DeviceContext &deviceContext,
            float depth,
            float fieldOfView,
            float aspectRatio
    ) {
        const auto &device = deviceContext.getDevice();
        auto layout = overeditor::utility::DescriptorLayout(
                {
                        overeditor::utility::DescriptorElement(
                                sizeof(CameraMatrices),
                                1,
                                vk::DescriptorType::eUniformBuffer
                        )
                }
        );
        std::vector<vk::DescriptorPoolSize> sizes = overeditor::utility::layouts::toSizes(
                layout
        );

        vk::DescriptorSetLayout dLayout = overeditor::utility::layouts::toDescriptorLayout(
                vk::ShaderStageFlagBits::eVertex,
                device, layout
        );
        auto mBuf = device.createBuffer(
                vk::BufferCreateInfo(
                        (vk::BufferCreateFlags) 0,
                        sizeof(CameraMatrices),
                        vk::BufferUsageFlagBits::eUniformBuffer
                )
        );
        auto bufMem = overeditor::utility::vulkan::createBufferMemory(deviceContext, mBuf);

        auto pool = device.createDescriptorPool(
                vk::DescriptorPoolCreateInfo(
                        (vk::DescriptorPoolCreateFlags) 0,
                        1,
                        sizes.size(), sizes.data()
                )
        );
        auto sets = device.allocateDescriptorSets(
                vk::DescriptorSetAllocateInfo(
                        pool,
                        1,
                        &dLayout
                )
        );
        return Camera(
                depth, fieldOfView, aspectRatio, mBuf, bufMem, dLayout, sets[0], pool
        );
    }

    Camera(
            float depth,
            float fieldOfView,
            float aspectRatio,
            vk::Buffer matricesBuffer,
            vk::DeviceMemory memory,
            vk::DescriptorSetLayout layout,
            vk::DescriptorSet matricesSet,
            vk::DescriptorPool pool
    );
};

struct Drawable {
public:
    const vk::CommandBuffer buf;
    const overeditor::graphics::shaders::Shader *shader;
    const vk::DescriptorPool descriptorPool;
    const std::vector<vk::DescriptorSet> descriptors;

    static Drawable forGeometry(
            const overeditor::graphics::DeviceContext &deviceContext,
            const overeditor::graphics::shaders::Shader &shader,
            const vk::CommandPool &pool,
            const vk::RenderPass &renderPass,
            uint32_t subpass,
            const overeditor::graphics::GeometryBuffer &buffer
    );

    explicit Drawable(
            const vk::CommandBuffer &buf = nullptr,
            const vk::DescriptorPool &descriptorPool = nullptr,
            const overeditor::graphics::shaders::Shader *shader = nullptr,
            std::vector<vk::DescriptorSet> descriptors = std::vector<vk::DescriptorSet>()
    );
};

#endif //OVEREDITOR_COMMON_H

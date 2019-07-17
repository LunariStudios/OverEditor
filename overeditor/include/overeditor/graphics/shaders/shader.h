#ifndef OVEREDITOR_SHADER_H
#define OVEREDITOR_SHADER_H

#include <vulkan/vulkan.hpp>
#include <stdint.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <overeditor/graphics/device_context.h>

namespace overeditor::graphics::shaders {
    class ShaderSource {
    private:
        std::vector<char> buf;
    public:
        explicit ShaderSource(
                const std::filesystem::path &filepath
        );

        vk::ShaderModule createModuleFor(const vk::Device &device);

    };

#define PIPELINE_NAME "main"

    class Shader {
    private:
        const vk::Device *owner;
        ShaderSource *fragment, *vertex;
        vk::ShaderModule fragModule, vertModule;
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
    public:
        Shader();

        ~Shader();

        void initialize(
                const DeviceContext &deviceCtx,
                const vk::RenderPass &renderPass,
                const std::filesystem::path &fragmentPath,
                const std::filesystem::path &vertexPath
        );

        ShaderSource *getFragment() const;

        ShaderSource *getVertex() const;

        const vk::Pipeline &getPipeline() const;

    };

    class ShaderInstance {
    private:
        const Shader *parent;
        const vk::CommandBuffer buffer;
    public:

    };
}
#endif

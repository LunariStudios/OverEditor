#ifndef OVEREDITOR_SHADER_H
#define OVEREDITOR_SHADER_H

#include <vulkan/vulkan.hpp>
#include <stdint.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <overeditor/graphics/device_context.h>
#include <overeditor/graphics/buffers/vertices.h>

namespace overeditor {

    using DescriptorLayout = overeditor::DescriptorLayout;
    using VertexLayout = overeditor::VertexLayout;
    using PushConstantsLayout = overeditor::PushConstantsLayout;

    /**
     * Represents the SPIR-V code of a shader, including layout information, usually useful for descriptor sets
     */
    class ShaderSource {
    private:

        /**
         * SPIR-V code.
         */
        std::vector<char> buf;
        /**
         * A vector of layouts where each element co-relates to a descriptor set layout.
         * For example, you may have two layouts, one for the CameraMatrices descriptor set,
         * and another for the shader uniforms.
         */
        std::vector<DescriptorLayout> descriptorLayouts;
        VertexLayout shaderLayout;
        PushConstantsLayout pushConstantsLayout;
    public:
        ShaderSource(
                const std::filesystem::path &filepath,
                std::vector<DescriptorLayout> layout,
                VertexLayout shaderLayout,
                PushConstantsLayout pushConstantsLayout
        );

        const PushConstantsLayout &getPushConstantsLayout() const;

        vk::ShaderModule createModuleFor(const vk::Device &device) const;

        const std::vector<DescriptorLayout> &getDescriptorLayouts() const;

        const VertexLayout &getShaderLayout() const;
    };

#define PIPELINE_NAME "main"

    /**
     * Represents shader pair (vertex and fragment) loaded into a device, ready to be used to render an object.
     */
    class Shader {
    private:
        const std::string name;
        const vk::Device *owner;
        ShaderSource fragSource, vertSource;
        vk::ShaderModule fragModule, vertModule;
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        std::vector<vk::DescriptorSetLayout> descriptorsLayouts;
    public:
        Shader(
                std::string name,
                const ShaderSource &fragment,
                const ShaderSource &vertex,
                const DeviceContext &deviceCtx,
                const vk::RenderPass &renderPass
        );

        ~Shader();

        const vk::Pipeline &getPipeline() const;

        const vk::PipelineLayout &getLayout() const;

        const ShaderSource &getFragSource() const;

        const ShaderSource &getVertSource() const;

        const std::string &getName() const;

        const std::vector<vk::DescriptorSetLayout> &getDescriptorsLayouts() const;

        const std::vector<const DescriptorLayout *> &getDescriptors() const {
            std::vector<const DescriptorLayout *> r;
            auto selector = [&](const overeditor::DescriptorLayout &l) {
                return &l;
            };
            auto &vd = vertSource.getDescriptorLayouts();
            std::transform(
                    vd.begin(), vd.end(), std::back_inserter(r), selector
            );
            auto &fd = fragSource.getDescriptorLayouts();
            std::transform(
                    fd.begin(), fd.end(), std::back_inserter(r), selector
            );
            return r;
        }
    };


}
#endif

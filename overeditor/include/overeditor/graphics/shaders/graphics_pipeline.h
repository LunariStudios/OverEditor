#ifndef OVEREDITOR_GRAPHICSPIPELINE_H
#define OVEREDITOR_GRAPHICSPIPELINE_H

#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/graphics/device_context.h>
#include <overeditor/utility/collection_utility.h>
#include <overeditor/utility/vulkan_utility.h>
#include <iostream>

#define PIPELINE_NAME "main"
namespace overeditor::graphics::shaders {
    class GraphicsPipeline {
    private:
        Shader fragShader, vertShader;
        vk::ShaderModule fragModule, vertModule;
        vk::PipelineLayout layout;
        vk::RenderPass renderPass;
        vk::Pipeline pipeline;
        const vk::Device *devicePtr;
    public:
        GraphicsPipeline(
                const DeviceContext &deviceContext,
                const std::filesystem::path &resourceDir
        );

        ~GraphicsPipeline();

        const Shader &getFragShader() const;

        const Shader &getVertShader() const;

        const vk::ShaderModule &getFragModule() const;

        const vk::ShaderModule &getVertModule() const;

        const vk::PipelineLayout &getLayout() const;

        const vk::RenderPass &getRenderPass() const;

        const vk::Pipeline &getPipeline() const;
    };

}
#endif //OVEREDITOR_GRAPHICSPIPELINE_H

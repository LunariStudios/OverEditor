#include <overeditor/graphics/shaders/graphics_pipeline.h>

namespace overeditor::graphics::shaders {

    GraphicsPipeline::GraphicsPipeline(
            const DeviceContext &deviceContext,
            const std::filesystem::path &resourceDir
    ) : fragShader(resourceDir / "frag.spv"), vertShader(resourceDir / "vert.spv"), fragModule(), vertModule() {
        const auto &device = deviceContext.getDevice();
        devicePtr = &device;
        fragModule = fragShader.createShaderFor(device);
        vertModule = vertShader.createShaderFor(device);
        vk::PipelineShaderStageCreateInfo fragment(
                (vk::PipelineShaderStageCreateFlags) 0, //Flags
                vk::ShaderStageFlagBits::eFragment,// Stage
                fragModule,  // Module
                PIPELINE_NAME //Name
        );
        vk::PipelineShaderStageCreateInfo vertex(
                (vk::PipelineShaderStageCreateFlags) 0, //Flags
                vk::ShaderStageFlagBits::eVertex,// Stage
                vertModule,  // Module
                PIPELINE_NAME //Name
        );
        vk::PipelineShaderStageCreateInfo shaderStages[2] = {
                vertex, fragment
        };
        auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo();
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
                (vk::PipelineInputAssemblyStateCreateFlags) 0,
                vk::PrimitiveTopology::eTriangleList
        );
        auto extent = deviceContext.getSwapChainContext()->getSwapchainExtent();
        // Viewport Stage
        vk::Viewport viewport(0, 0, extent.width, extent.height, 0, 1);
        vk::Rect2D scissor(vk::Offset2D(), extent);
        vk::PipelineViewportStateCreateInfo viewportInfo(
                (vk::PipelineViewportStateCreateFlags) 0,
                1, &viewport, 1, &scissor
        );
        vk::PipelineRasterizationStateCreateInfo rasterizer(
                (vk::PipelineRasterizationStateCreateFlags) 0,
                VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise,
                VK_FALSE, 0, 0, 0, 1
        );
        vk::PipelineMultisampleStateCreateInfo multisampler(
                (vk::PipelineMultisampleStateCreateFlags) 0,
                vk::SampleCountFlagBits::e1,
                VK_FALSE,
                1,
                nullptr,
                VK_FALSE,
                VK_FALSE
        );

        vk::PipelineColorBlendAttachmentState colorBlend(
                VK_FALSE,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::BlendFactor::eOne,
                vk::BlendFactor::eZero,
                vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eA | vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB
        );
        vk::PipelineColorBlendStateCreateInfo colorBlending(
                (vk::PipelineColorBlendStateCreateFlags) 0,
                VK_FALSE, vk::LogicOp::eCopy,
                1, &colorBlend
        );
        vk::DynamicState dynamicStates[] = {
                vk::DynamicState::eViewport,
                vk::DynamicState::eLineWidth
        };
        vk::PipelineDynamicStateCreateInfo dynamicState(
                (vk::PipelineDynamicStateCreateFlags) 0,
                2,
                dynamicStates
        );
        auto layoutInfo = vk::PipelineLayoutCreateInfo();
        vkAssertOk(vkCreatePipelineLayout(
                device,
                reinterpret_cast<VkPipelineLayoutCreateInfo *>(&layoutInfo),
                nullptr,
                reinterpret_cast<VkPipelineLayout *>(&layout)
        ));

        vk::AttachmentDescription colorAttachment(
                (vk::AttachmentDescriptionFlags) 0,
                deviceContext.getSwapChainContext()->getSwapchainFormat(),
                vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear,
                vk::AttachmentStoreOp::eStore,
                vk::AttachmentLoadOp::eDontCare,
                vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined,
                vk::ImageLayout::ePresentSrcKHR
        );
        vk::AttachmentReference colorAttachmentRef(
                0,
                vk::ImageLayout::eColorAttachmentOptimal
        );
        vk::SubpassDescription subpass(
                (vk::SubpassDescriptionFlags) 0,
                vk::PipelineBindPoint::eGraphics,
                0, nullptr,
                1, &colorAttachmentRef
        );
        vk::RenderPassCreateInfo renderPassCreateInfo(
                (vk::RenderPassCreateFlags) 0,
                1,
                &colorAttachment,
                1,
                &subpass
        );
        vkAssertOk(
                vkCreateRenderPass(
                        device,
                        reinterpret_cast<VkRenderPassCreateInfo *>(&renderPassCreateInfo),
                        nullptr,
                        reinterpret_cast<VkRenderPass *>(&renderPass)
                )
        )
        vk::GraphicsPipelineCreateInfo graphicsInfo(
                (vk::PipelineCreateFlags) 0,
                2,// Stage count
                shaderStages, //
                &vertexInputInfo,
                &inputAssembly,
                nullptr,
                &viewportInfo,
                &rasterizer,
                &multisampler,
                nullptr,
                &colorBlending,
                &dynamicState, layout, renderPass, 0, nullptr, -1
        );
        vkAssertOk(
                vkCreateGraphicsPipelines(
                        device,
                        VK_NULL_HANDLE,
                        1,
                        reinterpret_cast<VkGraphicsPipelineCreateInfo *>(&graphicsInfo),
                        nullptr,
                        reinterpret_cast<VkPipeline *>(&pipeline)
                )
        )
    }

    GraphicsPipeline::~GraphicsPipeline() {
        const vk::Device &device = *devicePtr;
        vkDestroyPipeline(device, pipeline, nullptr);
        vkDestroyPipelineLayout(device, layout, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);
        vkDestroyShaderModule(device, fragModule, nullptr);
        vkDestroyShaderModule(device, vertModule, nullptr);
    }

    const Shader &GraphicsPipeline::getFragShader() const {
        return fragShader;
    }

    const Shader &GraphicsPipeline::getVertShader() const {
        return vertShader;
    }

    const vk::ShaderModule &GraphicsPipeline::getFragModule() const {
        return fragModule;
    }

    const vk::ShaderModule &GraphicsPipeline::getVertModule() const {
        return vertModule;
    }

    const vk::PipelineLayout &GraphicsPipeline::getLayout() const {
        return layout;
    }

    const vk::RenderPass &GraphicsPipeline::getRenderPass() const {
        return renderPass;
    }

    const vk::Pipeline &GraphicsPipeline::getPipeline() const {
        return pipeline;
    }


}
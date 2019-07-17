#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/utility/vulkan_utility.h>

namespace overeditor::graphics::shaders {

    ShaderSource::ShaderSource(const std::filesystem::path &filepath) : buf() {
        std::ifstream file(filepath, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error(std::string("Unable to open file: ") + filepath.string());
        }
        size_t fileSize = (size_t) file.tellg();
        buf.resize(fileSize);
        file.seekg(SEEK_SET);
        file.read(buf.data(), fileSize);
        file.close();
    }

    vk::ShaderModule ShaderSource::createModuleFor(const vk::Device &device) {
        VkShaderModuleCreateInfo info;
        info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        info.pNext = nullptr;
        info.codeSize = buf.size();
        info.flags = 0;
        info.pCode = reinterpret_cast<uint32_t *>( buf.data());

        vk::ShaderModule mod;
        vkAssertOk(vkCreateShaderModule(
                device,
                reinterpret_cast<VkShaderModuleCreateInfo *>(&info),
                nullptr,
                reinterpret_cast<VkShaderModule *>(&mod)
        ));
        return mod;
    }

    Shader::Shader() : owner(nullptr), fragment(nullptr), vertex(nullptr) {

    }

    void Shader::initialize(
            const DeviceContext &deviceCtx,
            const vk::RenderPass &renderPass,
            const std::filesystem::path &fragmentPath,
            const std::filesystem::path &vertexPath
    ) {
        auto device = deviceCtx.getDevice();
        owner = &device;
        fragment = new ShaderSource(fragmentPath);
        vertex = new ShaderSource(vertexPath);
        fragModule = fragment->createModuleFor(device);
        vertModule = vertex->createModuleFor(device);
        vk::PipelineShaderStageCreateInfo fragmentInfo(
                (vk::PipelineShaderStageCreateFlags) 0, //Flags
                vk::ShaderStageFlagBits::eFragment,// Stage
                fragModule,  // Module
                PIPELINE_NAME //Name
        );
        vk::PipelineShaderStageCreateInfo vertexInfo(
                (vk::PipelineShaderStageCreateFlags) 0, //Flags
                vk::ShaderStageFlagBits::eVertex,// Stage
                vertModule,  // Module
                PIPELINE_NAME //Name
        );
        vk::PipelineShaderStageCreateInfo shaderStages[2] = {
                vertexInfo, fragmentInfo
        };
        auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
                (vk::PipelineVertexInputStateCreateFlags) 0
        );
        vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
                (vk::PipelineInputAssemblyStateCreateFlags) 0,
                vk::PrimitiveTopology::eTriangleList
        );
        auto extent = deviceCtx.getSwapChainContext()->getSwapchainExtent();
        // Viewport Stage
        vk::Viewport viewport(
                0, 0,
                extent.width, extent.height,
                0, 1
        );
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

        layout = device.createPipelineLayout(
                vk::PipelineLayoutCreateInfo(
                        (vk::PipelineLayoutCreateFlags) 0
                )
        );


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
                nullptr, layout, renderPass, 0, nullptr, -1
        );
        pipeline = device.createGraphicsPipeline(nullptr, graphicsInfo);
    }

    const vk::Pipeline &Shader::getPipeline() const {
        return pipeline;
    }

    Shader::~Shader() {
        owner->destroy(pipeline);
        owner->destroy(layout);
        owner->destroy(fragModule);
        owner->destroy(vertModule);
    }

    ShaderSource *Shader::getFragment() const {
        return fragment;
    }

    ShaderSource *Shader::getVertex() const {
        return vertex;
    }
}
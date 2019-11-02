#include <utility>

#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/utility/vulkan_utility.h>
#include <overeditor/ecs/components/common.h>

namespace overeditor {

    void import_layouts(
            const ShaderSource &source,
            const vk::Device &device,
            const vk::ShaderStageFlags &stage,
            std::vector<vk::DescriptorSetLayout> &output
    ) {
        for (auto &layout:source.getDescriptorLayouts()) {
            output.push_back(
                    overeditor::layouts::toDescriptorSetLayout(
                            stage,
                            device,
                            layout
                    )
            );
        }
    }

    void import_push_constants(
            const ShaderSource &source,
            const vk::ShaderStageFlags &stage,
            std::vector<vk::PushConstantRange> &output
    ) {
        auto &constants = source.getPushConstantsLayout().getElements();
        size_t offset = 0;
        output.reserve(constants.size());
        for (const overeditor::LayoutElement &element : constants) {
            size_t size = element.getSize();
            output.emplace_back(
                    stage,
                    offset,
                    size
            );
            offset += size;
        }
    }

    ShaderIndices::ShaderIndices(
            uint32_t cameraMatricesIndex,
            uint32_t modelMatrix
    ) : cameraMatricesIndex(cameraMatricesIndex), modelMatrix(modelMatrix) {

    }
    ShaderSource::ShaderSource(
            const std::filesystem::path &filepath,
            std::vector<DescriptorLayout> layouts,
            VertexLayout shaderLayout,
            PushConstantsLayout pushConstantsLayout
    ) : buf(),
        shaderLayout(std::move(shaderLayout)),
        descriptorLayouts(std::move(layouts)),
        pushConstantsLayout(std::move(pushConstantsLayout)) {
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

    vk::ShaderModule ShaderSource::createModuleFor(
            const vk::Device &device
    ) const {
        return device.createShaderModule(
                vk::ShaderModuleCreateInfo(
                        (vk::ShaderModuleCreateFlags) 0,
                        buf.size(),
                        reinterpret_cast<const uint32_t *>( buf.data())
                )
        );
    }

    const std::vector<DescriptorLayout> &ShaderSource::getDescriptorLayouts() const {
        return descriptorLayouts;
    }

    const VertexLayout &ShaderSource::getShaderLayout() const {
        return shaderLayout;
    }

    const PushConstantsLayout &ShaderSource::getPushConstantsLayout() const {
        return pushConstantsLayout;
    }


    Shader::Shader(
            std::string name,
            ShaderIndices indices,
            ShaderSource fragment,
            ShaderSource vertex,
            const DeviceContext &deviceCtx,
            const vk::RenderPass &renderPass
    ) : name(std::move(name)),
        indices(indices),
        owner(nullptr),
        descriptorsLayouts(),
        fragSource(std::move(fragment)), vertSource(std::move(vertex)) {
        // First phase - Assign members
        owner = deviceCtx.getDevice();
        fragModule = fragSource.createModuleFor(owner);
        vertModule = vertSource.createModuleFor(owner);
        // Second phase - Init vulkan stuff
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
        std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = {
                vertexInfo, fragmentInfo
        };

        auto &vLayout = vertex.getShaderLayout();
        std::vector<vk::VertexInputBindingDescription> vertexBindings = {
                vk::VertexInputBindingDescription(
                        0,
                        static_cast<uint32_t >(vLayout.getStride()),
                        vk::VertexInputRate::eVertex
                )
        };
        std::vector<vk::VertexInputAttributeDescription> vertexAttributes;
        auto &elements = vLayout.getElements();
        vertexAttributes.reserve(elements.size());
        for (size_t i = 0; i < elements.size(); ++i) {
            const auto &element = elements[i];
            LOG_INFO << "Pushing Vertex element " << i << " VertexElement(" << element << ")";
            vertexAttributes.emplace_back(
                    i,
                    0,
                    element.getFormat(), 0

            );
        }
        auto vertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
                (vk::PipelineVertexInputStateCreateFlags) 0,
                vertexBindings.size(), vertexBindings.data(),
                vertexAttributes.size(), vertexAttributes.data()
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
        import_layouts(
                vertex, owner,
                vk::ShaderStageFlagBits::eVertex,
                descriptorsLayouts
        );
        import_layouts(
                fragment, owner,
                vk::ShaderStageFlagBits::eFragment,
                descriptorsLayouts
        );
        std::vector<vk::PushConstantRange> ranges;
        import_push_constants(
                vertex,
                vk::ShaderStageFlagBits::eVertex,
                ranges
        );
        import_push_constants(
                fragment,
                vk::ShaderStageFlagBits::eFragment,
                ranges
        );
        layout = owner.createPipelineLayout(
                vk::PipelineLayoutCreateInfo(
                        (vk::PipelineLayoutCreateFlags) 0,
                        descriptorsLayouts.size(), descriptorsLayouts.data(),
                        ranges.size(), ranges.data()
                )
        );

        vk::GraphicsPipelineCreateInfo graphicsInfo(
                (vk::PipelineCreateFlags) 0,
                shaderStages.size(),// Stage count
                shaderStages.data(), //
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
        pipeline = owner.createGraphicsPipeline(nullptr, graphicsInfo);
    }

    const vk::Pipeline &Shader::getPipeline() const {
        return pipeline;
    }

    Shader::~Shader() {
        owner.destroy(pipeline);
        owner.destroy(layout);
        owner.destroy(fragModule);
        owner.destroy(vertModule);
        for (auto &l : descriptorsLayouts) {
            owner.destroy(l);
        }
    }

    const vk::PipelineLayout &Shader::getLayout() const {
        return layout;
    }

    const ShaderSource &Shader::getFragSource() const {
        return fragSource;
    }

    const ShaderSource &Shader::getVertSource() const {
        return vertSource;
    }

    const std::vector<vk::DescriptorSetLayout> &Shader::getDescriptorsLayouts() const {
        return descriptorsLayouts;
    }

    const std::string &Shader::getName() const {
        return name;
    }

    const ShaderIndices &Shader::getIndices() const {
        return indices;
    }

}
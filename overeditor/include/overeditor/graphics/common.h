#ifndef OVEREDITOR_GRAPHICS_COMMON_H
#define OVEREDITOR_GRAPHICS_COMMON_H

#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/graphics/descriptors.h>

namespace overeditor {
    inline vk::WriteDescriptorSet updateModelMatrixWrite(
            const overeditor::DescriptorsController &descriptorsController,
            const overeditor::Shader &shader
    ) {
        auto dstIndex = shader.getIndices().modelMatrix;
        auto destination = descriptorsController.getDescriptors()[dstIndex];
        auto buffer = descriptorsController.getDescriptorsBuffers()[dstIndex];
        vk::DescriptorBufferInfo info(
                buffer, 0, 0
        );
        LOG_INFO << "Writing to " << destination << " @ " << buffer;
        return vk::WriteDescriptorSet(
                destination,
                0,
                1,
                1,
                vk::DescriptorType::eUniformBuffer,
                nullptr,
                &info
        );
    }
}
#endif

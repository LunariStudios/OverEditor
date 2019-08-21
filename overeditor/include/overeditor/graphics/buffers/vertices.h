#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#include <utility>

#ifndef OVEREDITOR_VERTICES_H
#define OVEREDITOR_VERTICES_H

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/device_context.h>
#include <overeditor/utility/memory_layout.h>
#include <overeditor/utility/vulkan_utility.h>

namespace overeditor {

    using VertexLayout=overeditor::VertexLayout;

    class GeometryBuffer {
    private:
        const VertexLayout layout;
        size_t totalIndices;
        const vk::Device *host;
        const vk::Buffer vertexBuffer, indexBuffer;
        const vk::DeviceMemory vertexMemory, indexMemory;

    public:
        GeometryBuffer(
                VertexLayout layout,
                const size_t totalIndices,
                const vk::Device *host,
                const vk::Buffer &vertexBuffer,
                const vk::Buffer &indexBuffer,
                const vk::DeviceMemory &vertexMemory,
                const vk::DeviceMemory &indexMemory
        ) : layout(std::move(layout)), host(host), totalIndices(totalIndices),
            vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
            vertexMemory(vertexMemory), indexMemory(indexMemory) {}

        static GeometryBuffer create(
                const VertexLayout &layout,
                const DeviceContext &deviceContext,
                void *vertexData,
                uint32_t vertexCount,
                uint16_t *indexData,
                uint32_t indexCount
        ) {
            const auto &device = deviceContext.getDevice();
            size_t vSize = vertexCount * layout.getStride();
            vk::Buffer vBuf;
            vk::DeviceMemory vMem;
            overeditor::createAndPrepareBuffer(
                    deviceContext,
                    vSize,
                    vBuf,
                    vMem,
                    vk::BufferUsageFlagBits::eVertexBuffer,
                    vertexData
            );
            size_t iSize = indexCount * sizeof(uint16_t);

            vk::Buffer iBuf;
            vk::DeviceMemory iMem;
            overeditor::createAndPrepareBuffer(
                    deviceContext,
                    iSize,
                    iBuf,
                    iMem,
                    vk::BufferUsageFlagBits::eIndexBuffer,
                    vertexData
            );
            const vk::Device *devicePtr = &device;
            return GeometryBuffer(
                    layout, indexCount, devicePtr,
                    vBuf, iBuf,
                    vMem, iMem
            );
        }

        size_t getTotalIndices() const {
            return totalIndices;
        }

        const vk::Buffer &getVertexBuffer() const {
            return vertexBuffer;
        }

        const vk::Buffer &getIndexBuffer() const {
            return indexBuffer;
        }
    };

}


#endif

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

    /**
     * Holds all the data required for a object to be drawn
     */
    struct GeometryBuffer {
    private:
        /**
         * Holds the data layout for a single vertex
         */
        const VertexLayout layout;

        /**
         * How many indices are there?
         */
        size_t totalIndices;

        /**
         * The device which will be used to draw this geometry
         */
        const vk::Device host;
        const vk::Buffer vertexBuffer, indexBuffer;
        const vk::DeviceMemory vertexMemory, indexMemory;

    public:
        GeometryBuffer(
                VertexLayout layout,
                const size_t totalIndices,
                const vk::Device host,
                const vk::Buffer &vertexBuffer,
                const vk::Buffer &indexBuffer,
                const vk::DeviceMemory &vertexMemory,
                const vk::DeviceMemory &indexMemory
        ) : layout(std::move(layout)), host(host), totalIndices(totalIndices),
            vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
            vertexMemory(vertexMemory), indexMemory(indexMemory) {}

        /**
         * Uses the given deviceContext to allocate required resources for a new GeometryBuffer.
         * @param layout The vertex layout of the geometry
         * @param deviceContext
         * @param vertexData
         * @param vertexCount
         * @param indexData
         * @param indexCount
         * @return
         */
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
            return GeometryBuffer(
                    layout, indexCount, device,
                    vBuf, iBuf,
                    vMem, iMem
            );
        }

        size_t getTotalIndices() const {
            return totalIndices;
        }

        vk::Buffer getVertexBuffer() const {
            return vertexBuffer;
        }

        vk::Buffer getIndexBuffer() const {
            return indexBuffer;
        }
    };

}


#endif

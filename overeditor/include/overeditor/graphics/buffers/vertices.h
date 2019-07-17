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

namespace overeditor::graphics {

    class GeometryLayoutElement {
    private:
        uint8_t elementLength;
        uint8_t elementCount;
    public:
        GeometryLayoutElement(uint8_t elementLength, uint8_t elementCount) : elementLength(elementLength),
                                                                             elementCount(elementCount) {}

        uint8_t getElementLength() const {
            return elementLength;
        }

        uint8_t getElementCount() const {
            return elementCount;
        }

        uint8_t getSize() const {
            return elementLength * elementCount;
        }
    };

    class GeometryLayout {
    private:
        std::vector<GeometryLayoutElement> elements;
    public:
        template<typename T>
        void push(uint8_t count);


        uint8_t getStride() const {
            uint8_t s = 0;
            for (auto &e : elements) {
                s += e.getSize();
            }
            return s;
        }


    };


    class GeometryBuffer {
    private:
        GeometryLayout layout;
        vk::Buffer buffer;
        vk::DeviceMemory memory;

    public:
        explicit GeometryBuffer(
                GeometryLayout layout
        ) : layout(std::move(layout)) {}

        void dispose(vk::Device &device) {
            device.destroy(buffer);
            device.free(memory);
        }


    };

}


#endif

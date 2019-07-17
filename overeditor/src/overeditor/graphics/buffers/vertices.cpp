#include <overeditor/graphics/buffers/vertices.h>

namespace overeditor::graphics {
    template<>
    void GeometryLayout::push<float>(uint8_t count) {
        elements.emplace_back(sizeof(float), count);
    }

    template<>
    void GeometryLayout::push<uint8_t>(uint8_t count) {
        elements.emplace_back(sizeof(uint8_t), count);
    }

    template<>
    void GeometryLayout::push<uint32_t>(uint8_t count) {
        elements.emplace_back(sizeof(uint32_t), count);
    }
}
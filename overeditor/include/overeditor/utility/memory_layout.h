#include <cstdint>
#include <vector>
#include <vulkan/vulkan.hpp>
#include <ostream>
#include <overeditor/utility/string_utility.h>

#ifndef OVEREDITOR_MEMORY_LAYOUT_H
#define OVEREDITOR_MEMORY_LAYOUT_H
namespace overeditor {

    class LayoutElement {
    private:
        /**
         * The bytes length of a single item.
         */
        uint8_t elementLength;
        /**
         * How many items are in this element.
         */
        uint8_t elementCount;
    public:
        LayoutElement(
                uint8_t elementLength,
                uint8_t elementCount
        ) : elementLength(elementLength),
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

        friend std::ostream &operator<<(std::ostream &os, const LayoutElement &element);
    };


    template<typename E>
    class Layout {
    private:
        std::vector<E> elements;
    public:

        Layout(
                std::initializer_list<E> elements = {}
        ) : elements(elements) {}

        const std::vector<E> &getElements() const {
            return elements;
        }

        uint8_t getStride() const {
            uint8_t s = 0;
            for (auto &e : elements) {
                s += e.getSize();
            }
            return s;
        }


    };

    class DescriptorElement : public LayoutElement {
    private:
        /**
         * The type of this element when used within Vulkan.
         */
        vk::DescriptorType type;
        /**
         * Whether the descriptor set used to provide this element will be provided by a thirdparty.
         * For example: A Camera may provide the matrices descriptor set used by shaders.
         */
        bool imported;
    public:
        DescriptorElement(
                uint8_t elementLength,
                uint8_t elementCount,
                vk::DescriptorType type,
                bool imported = false
        );

        vk::DescriptorType getType() const;

        bool isImported() const;

    };

    class VertexElement : public LayoutElement {
    private:
        vk::Format format;
    public:
        VertexElement(
                uint8_t elementLength,
                uint8_t elementCount,
                vk::Format format
        );

        vk::Format getFormat() const;

        friend std::ostream &operator<<(std::ostream &os, const VertexElement &element);
    };

    typedef Layout<DescriptorElement> DescriptorLayout;
    typedef Layout<VertexElement> VertexLayout;
    typedef Layout<LayoutElement> PushConstantsLayout;

    template<typename T>
    std::string to_string(const Layout<T> layout) {
        std::string str = "Layout<";
        str += typeid(T).name();
        str += ">(";
        str += "stride: ";
        str += std::to_string(layout.getStride());
        str += ", elements: ";
        str += std::to_string(layout.getElements().size());
        str += ")";
        return str;
    }

    namespace layouts {
        vk::DescriptorSetLayout toDescriptorSetLayout(
                const vk::ShaderStageFlags &stage,
                const vk::Device &device,
                const DescriptorLayout &layout
        );

        std::vector<vk::DescriptorPoolSize> toSizes(
                const DescriptorLayout &layout
        );
    }
}
#endif

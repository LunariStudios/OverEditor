#include <overeditor/utility/memory_layout.h>
#include <plog/Log.h>

namespace overeditor {


    vk::DescriptorType DescriptorElement::getType() const {
        return type;
    }

    bool DescriptorElement::isImported() const {
        return imported;
    }

    DescriptorElement::DescriptorElement(
            uint8_t elementLength,
            uint8_t elementCount,
            vk::DescriptorType type,
            bool imported
    ) : LayoutElement(elementLength, elementCount),
        type(type), imported(imported) {}

    namespace layouts {
        std::vector<vk::DescriptorPoolSize> toSizes(const DescriptorLayout &layout) {
            std::vector<vk::DescriptorPoolSize> vector;
            const auto &elements = layout.getElements();
            std::transform(
                    elements.begin(), elements.end(),
                    std::back_inserter(vector),
                    [](const DescriptorElement &element) {
                        return vk::DescriptorPoolSize(
                                element.getType(),
                                element.getElementCount()
                        );
                    }
            );
            return vector;
        }

        vk::DescriptorSetLayout toDescriptorSetLayout(
                const vk::ShaderStageFlags &stage,
                const vk::Device &device,
                const DescriptorLayout &layout
        ) {
            const auto &elements = layout.getElements();
            auto end = elements.size();
            std::vector<vk::DescriptorSetLayoutBinding> bindings;

            for (size_t i = 0; i < end; i++) {
                const auto &e = elements[i];
                LOG_INFO << "Adding binding at position " << i << " (" << vk::to_string(e.getType()) << "("
                         << (int) e.getElementLength() <<
                         ") x "
                         << (int) e.getElementCount() << ")";
                bindings.emplace_back(
                        i,
                        e.getType(),
                        e.getElementCount(),
                        stage
                );

            }
            return device.createDescriptorSetLayout(
                    vk::DescriptorSetLayoutCreateInfo(
                            (vk::DescriptorSetLayoutCreateFlags) 0,
                            bindings.size(), bindings.data()
                    )
            );
        }
    }

    vk::Format VertexElement::getFormat() const {
        return format;
    }

    VertexElement::VertexElement(uint8_t elementLength, uint8_t elementCount, vk::Format format) : LayoutElement(
            elementLength, elementCount), format(format) {}

    std::ostream &operator<<(std::ostream &os, const VertexElement &element) {
        os << static_cast<const LayoutElement &>(element) << ", format: " << BYTE_STRING(element.getFormat()) << " @ "
           << vk::to_string(element.format);
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const LayoutElement &element) {
        os << "elementLength: " << BYTE_STRING(element.elementLength) << ", elementCount: "
           << BYTE_STRING(element.elementCount);
        return os;
    }
}
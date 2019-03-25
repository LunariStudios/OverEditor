#ifndef OVEREDITOR_IMAGE_CONTEXT_H
#define OVEREDITOR_IMAGE_CONTEXT_H

#include <vulkan/vulkan.hpp>

namespace overeditor::graphics {

    class ImageContext {
    private:
        vk::Image image;
        vk::ImageView view;
    public:
        ImageContext(
                const vk::Image &image,
                const vk::ImageView &imageView
        );

        const vk::Image &getImage() const;

        const vk::ImageView &getView() const;

    };
}
#endif

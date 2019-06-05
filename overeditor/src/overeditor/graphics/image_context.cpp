#include <overeditor/graphics/image_context.h>

namespace overeditor::graphics {
    ImageContext::ImageContext(
            const vk::Image &image,
            const vk::ImageView &imageView
    ) : image(image), view(imageView) {}

    const vk::Image &ImageContext::getImage() const {
        return image;
    }

    const vk::ImageView &ImageContext::getView() const {
        return view;
    }

}


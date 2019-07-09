#ifndef OVEREDITOR_SHADER_H
#define OVEREDITOR_SHADER_H

#include <vulkan/vulkan.hpp>
#include <stdint.h>
#include <filesystem>
#include <fstream>
#include <string>

namespace overeditor::graphics::shaders {
    class Shader {
    private:
        std::vector<char> buf;
    public:
        explicit Shader(
                const std::filesystem::path &filepath
        );

        vk::ShaderModule createShaderFor(const vk::Device &device);

    };
}
#endif

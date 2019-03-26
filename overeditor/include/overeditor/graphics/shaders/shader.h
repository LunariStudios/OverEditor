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
        std::vector<uint8_t> buf;
    public:
        explicit Shader(
                const std::filesystem::path &filepath
        ) {
            std::basic_ifstream<uint8_t> file(filepath, std::ios::ate | std::ios::binary);
            if (!file.is_open()) {
                throw std::runtime_error(std::string("Unable to open file: ") + filepath.string());
            }
            size_t fileSize = (size_t) file.tellg();
            buf.resize(fileSize);
            file.seekg(SEEK_SET);
            file.read(buf.data(), fileSize);
            file.close();
        }

        vk::ShaderModule createShaderFor(const vk::Device &device) {
            auto info = vk::ShaderModuleCreateInfo(
                    (vk::ShaderModuleCreateFlags) 0,
                    buf.size(),
                    reinterpret_cast<uint32_t *>(buf.data())
            );
            vk::ShaderModule mod;
            auto createShaderResult = vkCreateShaderModule(
                    device,
                    reinterpret_cast<VkShaderModuleCreateInfo *>(&info),
                    nullptr,
                    reinterpret_cast<VkShaderModule *>(&mod)
            );
            if (createShaderResult != VK_SUCCESS) {
                throw std::runtime_error(std::string("Unable to create shader: ") + vk::to_string((vk::Result) createShaderResult));
            }
        }

    };
}
#endif

#include <overeditor/graphics/shaders/shader.h>
#include <overeditor/utility/vulkan_utility.h>

namespace overeditor::graphics::shaders {
    Shader::Shader(const std::filesystem::path &filepath) {
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

    vk::ShaderModule Shader::createShaderFor(const vk::Device &device) {
        auto info = vk::ShaderModuleCreateInfo(
                (vk::ShaderModuleCreateFlags) 0,
                buf.size(),
                reinterpret_cast<uint32_t *>(buf.data())
        );
        vk::ShaderModule mod;
        vkAssertOk(vkCreateShaderModule(
                device,
                reinterpret_cast<VkShaderModuleCreateInfo *>(&info),
                nullptr,
                reinterpret_cast<VkShaderModule *>(&mod)
        ));
        return mod;
    }
}
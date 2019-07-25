#ifndef OVEREDITOR_VULKAN_UTILITY_H
#define OVEREDITOR_VULKAN_UTILITY_H

#include <vector>
#include <vulkan/vulkan.hpp>
#include <overeditor/graphics/device_context.h>

#define ENUMERATE_FUNC(func, type, vec, ...) uint32_t total;\
    func(__VA_ARGS__, &total, nullptr);\
    vec.resize(total);\
    func(device, surface, &total,reinterpret_cast<type *> (vec.data()));\


#define COMBINE_HELPER(X, Y) X##Y
#define COMBINE(X, Y) COMBINE_HELPER(X,Y)
#define vkAssertVar COMBINE(vkresult, __LINE__)
#define _vkAssertOk(x, error_msg)\
    auto vkAssertVar = x;\
    if((VkResult) vkAssertVar != VK_SUCCESS) { \
        throw std::runtime_error(error_msg);\
    }
#define vkAssertOkOr(x, error_msg) _vkAssertOk(x, std::string(error_msg) + ": " + vk::to_string((vk::Result) vkAssertVar) + ")")

#define vkAssertOk(x) _vkAssertOk(x, std::string("Error while executing ") + #x +": " + vk::to_string((vk::Result) vkAssertVar) + ")")
namespace overeditor::utility::vulkan {
    uint32_t getMemoryTypeIndex(
            const overeditor::graphics::DeviceContext &device,
            const vk::MemoryRequirements &requirements,
            const vk::MemoryPropertyFlags &flags
    );

    vk::DeviceMemory setupBufferMemory(
            const overeditor::graphics::DeviceContext &deviceContext,
            vk::Buffer &buffer,
            void *data
    );

    vk::DeviceMemory createBufferMemory(
            const overeditor::graphics::DeviceContext &deviceContext,
            vk::Buffer &buffer
    );

}
#endif

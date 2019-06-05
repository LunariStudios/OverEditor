#include <overeditor/utility/vulkan_utility.h>

#define ENUMERATE_FUNC(func, type, vec, ...) uint32_t total;\
    func(__VA_ARGS__, &total, nullptr);\
    vec.resize(total);\
    func(device, surface, &total,reinterpret_cast<type *> (vec.data()));\



set(CASC_BUILD_STATIC_LIB ON CACHE BOOL "Enable CASC static lib")
set(CASC_BUILD_SHARED_LIB OFF CACHE BOOL "Disable CASC shared lib")
set(VULKAN_STATIC_LIBRARY ${Vulkan_LIBRARY})
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "No GLFW examples")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "No GLFW tests")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "No GLFW Docs")
set(GLFW_VULKAN_STATIC ON CACHE BOOL "Enable static Vulkan Loader")


add_subdirectory(CascLib)
add_subdirectory(plog)
set(GLM_TEST_ENABLE OFF)
add_subdirectory(glm)
add_subdirectory(entityx)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)

add_subdirectory(glfw)

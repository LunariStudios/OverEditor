
set(
        OVEREDITOR_GRAPHICS
        include/overeditor/graphics/device_context.h
        src/overeditor/graphics/device_context.cpp

        include/overeditor/graphics/image_context.h
        src/overeditor/graphics/image_context.cpp

        include/overeditor/graphics/queue_context.h
        src/overeditor/graphics/queue_context.cpp

        include/overeditor/graphics/swapchain_context.h
        src/overeditor/graphics/swapchain_context.cpp

        include/overeditor/graphics/querying.h
        src/overeditor/graphics/querying.cpp

        include/overeditor/graphics/queue_families.h
        src/overeditor/graphics/queue_families.cpp

        include/overeditor/graphics/requirements.h
        src/overeditor/graphics/requirements.cpp
)
set(
        OVEREDITOR_COMMON
        include/overeditor/utility/collection_utility.h
        include/overeditor/utility/memory_utility.h
        include/overeditor/utility/step_function.h
        include/overeditor/utility/string_utility.h
        include/overeditor/utility/success_status.h
        include/overeditor/utility/vulkan_utility.h
        src/overeditor/utility/vulkan_utility.cpp
)
set(
        OVEREDITOR_APPLICATION
        include/overeditor/application.h
        src/overeditor/application.cpp
)
set(
        OVEREDITOR_MAIN src/overeditor/overeditor.cpp
)
set(
        OVEREDITOR_ALL
        ${OVEREDITOR_GRAPHICS}
        ${OVEREDITOR_COMMON}
        ${OVEREDITOR_APPLICATION}
        ${OVEREDITOR_MAIN}
)
add_executable(overeditor ${OVEREDITOR_ALL})
find_package(Vulkan REQUIRED)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")
get_target_property(VK_IDIR Vulkan::Vulkan INTERFACE_INCLUDE_DIRECTORIES)
target_link_libraries(overeditor casc_static Vulkan::Vulkan plog glfw)
target_include_directories(
        overeditor
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(
        overeditor
        PUBLIC
        OVEREDITOR_VERSION_MAJOR=${OVEREDITOR_VERSION_MAJOR}
        OVEREDITOR_VERSION_MINOR=${OVEREDITOR_VERSION_MINOR}
        OVEREDITOR_VERSION_PATCH=${OVEREDITOR_VERSION_PATCH}
)
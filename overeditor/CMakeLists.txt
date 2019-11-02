
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
        include/overeditor/graphics/shaders/shader.h src/overeditor/graphics/shaders/shader.cpp)
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
        include/overeditor/graphics/buffers/vertices.h
        include/overeditor/ecs/components/common.h
        include/overeditor/ecs/systems/rendering.h
        src/overeditor/graphics/buffers/vertices.cpp
        include/overeditor/utility/memory_layout.h
        src/overeditor/utility/memory_layout.cpp
        src/overeditor/ecs/systems/rendering.cpp
        src/overeditor/ecs/components/common.cpp
        include/overeditor/graphics/descriptors.h src/overeditor/graphics/descriptors.cpp include/overeditor/utility/singletons.h include/overeditor/graphics/common.h)
add_executable(overeditor ${OVEREDITOR_ALL})

set(
        OVEREDITOR_SHADERS
        res/shaders/standart.vert
        res/shaders/standart.frag
)

foreach (SHADER ${OVEREDITOR_SHADERS})
    list(APPEND SHADER_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
endforeach ()
if (WIN32)
    add_custom_command(
            TARGET overeditor
            PRE_BUILD
            COMMAND if not exist res mkdir res
    )
    add_custom_command(
            TARGET overeditor
            POST_BUILD
            COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator -V ${SHADER_PATHS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/res
    )
endif ()

if (UNIX)
    add_custom_command(
            TARGET overeditor
            PRE_BUILD
            COMMAND mkdir -p res
    )
    add_custom_command(
            TARGET overeditor
            POST_BUILD
            COMMAND $ENV{VULKAN_SDK}/bin/glslangValidator -V ${SHADER_PATHS}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/res
    )
    target_link_libraries(overeditor stdc++fs)
endif ()

find_package(Vulkan REQUIRED)

target_link_libraries(overeditor casc_static Vulkan::Vulkan plog glfw glm entityx)

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
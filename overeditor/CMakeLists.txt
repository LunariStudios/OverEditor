add_executable(overeditor
        src/overeditor/overeditor.cpp
        include/overeditor/application.h
        include/overeditor/utility/step_function.h
        src/overeditor/application.cpp
        include/overeditor/ui/rect.h
        include/overeditor/ui/panel.h
        )
find_package(Vulkan REQUIRED)

get_target_property(VK_IDIR Vulkan::Vulkan INTERFACE_INCLUDE_DIRECTORIES)
target_link_libraries(overeditor casc_static Vulkan::Vulkan)
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
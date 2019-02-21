add_executable(overeditor
        src/overeditor/overeditor.cpp
        include/overeditor/application.h
        include/overeditor/utility/step_function.h
        src/overeditor/application.cpp
        include/overeditor/ui/rect.h
        include/overeditor/ui/panel.h
        include/overeditor/utility/string_utility.h
        include/overeditor/utility/memory_utility.h
        include/overeditor/graphics/queue_families.h src/overeditor/graphics/queue_families.cpp include/overeditor/graphics/querying.h src/overeditor/graphics/querying.cpp include/overeditor/overeditor_constants.h include/overeditor/graphics/requirements.h src/overeditor/graphics/requirements.cpp include/overeditor/utility/collection_utility.h include/overeditor/utility/success_status.h)
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
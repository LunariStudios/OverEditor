
set(CASC_BUILD_STATIC_LIB ON CACHE BOOL "Enable CASC static lib")
set(CASC_BUILD_SHARED_LIB OFF CACHE BOOL "Disable CASC shared lib")

add_subdirectory(CascLib)

add_subdirectory(plog)
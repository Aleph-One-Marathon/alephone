set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_C_FLAGS -mmacosx-version-min=10.13)
set(VCPKG_CXX_FLAGS -mmacosx-version-min=10.13)

if(PORT STREQUAL "angle")
    set(VCPKG_LIBRARY_LINKAGE dynamic)
    set(NDEBUG 1)
    set(PORT_DEBUG OFF)
endif()

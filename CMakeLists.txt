cmake_minimum_required(VERSION 3.25.0)

project(NOCTURNE CXX)

file(GLOB_RECURSE SOURCE "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE HADERS "${PROJECT_SOURCE_DIR}/src/*.hpp" "${PROJECT_SOURCE_DIR}/src/*.h")

add_executable(nocturne ${SOURCE} ${HADERS})

set(DYNAMIC_LINK ON)
set(WEBGPU_BUILD_FROM_SOURCE ON)
set(WEBGPU_BACKEND "DAWN")

set_target_properties(nocturne PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

add_subdirectory("${PROJECT_SOURCE_DIR}/window/")
add_subdirectory("${PROJECT_SOURCE_DIR}/renderer/")

target_link_libraries(nocturne PRIVATE window renderer)

# copy binaries
target_copy_renderer_binaries(nocturne)
target_copy_window_binaries(nocturne)

target_include_directories(nocturne PRIVATE "${PROJECT_SOURCE_DIR}/include/" "${PROJECT_SOURCE_DIR}/vendor/")

set(CLANGD_CONFIG_FILE "${CMAKE_SOURCE_DIR}/.clangd")
configure_file(
    "${CMAKE_SOURCE_DIR}/.clangd.in"
    "${CLANGD_CONFIG_FILE}"
)

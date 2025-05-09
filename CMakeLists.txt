cmake_minimum_required(VERSION 3.25.0)

set(ENV{https_proxy} "http://127.0.0.1:7890")
set(ENV{http_proxy} "http://127.0.0.1:7890")

project(NOCTURNE CXX)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")

file(GLOB_RECURSE SOURCE "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.cc")

include(cmake/utils.cmake)
include(FetchContent)

list(APPEND BIN_FILES 
    "${CMAKE_SOURCE_DIR}/assets/wgsl/test.wgsl" 
    "${CMAKE_SOURCE_DIR}/assets/model/monkey_head.mtl" 
    "${CMAKE_SOURCE_DIR}/assets/model/monkey_head.obj"
)

foreach(BIN_FILE ${BIN_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${BIN_FILE})
    embed_binary_as_symbol(INPUT_FILE ${BIN_FILE} OUTPUT_OBJECT OBJ_OUTPUT SYMBOL_PREFIX "_binary_${REL_PATH}")
    list(APPEND BIN_OBJS ${OBJ_OUTPUT})
endforeach()

add_executable(nocturne ${BIN_OBJS} ${SOURCE} ${HADERS})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

set_target_properties(nocturne PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

FetchContent_Declare(
    assimp
    GIT_REPOSITORY  "https://github.com/assimp/assimp.git"
    GIT_TAG         "v5.4.3"
)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)

if(NOT MAVC)
    list(APPEND CMAKE_CXX_FLAGS "-Wno-nontrivial-memcall")
    list(APPEND CMAKE_C_FLAGS "-Wno-nontrivial-memcall")
endif()

FetchContent_MakeAvailable(assimp)

if(NOT MAVC)
    list(REMOVE_ITEM CMAKE_CXX_FLAGS "-Wno-nontrivial-memcall")
    list(REMOVE_ITEM CMAKE_C_FLAGS "-Wno-nontrivial-memcall")
endif()

set(WEBGPU_BACKEND "DAWN")
set(WINDOW_LINK_SHARED OFF)
set(RENDERER_LINK_SHARED OFF)

target_compile_options(nocturne PRIVATE -fexceptions)

add_subdirectory("${PROJECT_SOURCE_DIR}/window/")
add_subdirectory("${PROJECT_SOURCE_DIR}/renderer/")

target_link_libraries(nocturne PRIVATE webgpu assimp renderer window)

# copy binaries
target_copy_renderer_binaries(nocturne)
target_copy_window_binaries(nocturne)

target_include_directories(nocturne PRIVATE "${PROJECT_SOURCE_DIR}/include/" "${PROJECT_SOURCE_DIR}/vendor/")


list(APPEND IN_FILES "${CMAKE_SOURCE_DIR}/.clangd.in" "${CMAKE_SOURCE_DIR}/.vscode/launch.json.in")

foreach(IN_FILE ${IN_FILES})
    get_filename_component(OUTPUT_FILE_NAME ${IN_FILE} NAME_WLE)
    get_filename_component(OUTPUT_FILE_PATH ${IN_FILE} DIRECTORY)
    set(OUTPUT_FILE ${OUTPUT_FILE_PATH}/${OUTPUT_FILE_NAME})
    message(${OUTPUT_FILE})
    configure_file(
        ${IN_FILE}
        ${OUTPUT_FILE}
    )
endforeach(IN_FILE)

cmake_minimum_required(VERSION 3.25.0)

project(RENDERER CXX)

file(GLOB_RECURSE SOURCE "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.cc")
file(GLOB_RECURSE HEADER "${PROJECT_SOURCE_DIR}/src/*.hpp" "${PROJECT_SOURCE_DIR}/src/*.h")

if(DYNAMIC_LINK)
    message("-- ${PROJECT_NAME}: making shared lib")
    add_library(renderer SHARED ${SOURCE} ${HEADER})
    set(WEBGPU_LINK_TYPE "SHARED")
    if(WIN32)
        target_compile_definitions(renderer PRIVATE ${PROJECT_NAME}_LIB_EXPORTS)
    elseif(UNIX)
        target_compile_options(renderer PRIVATE -fvisibility=hidden)
    endif()
else()
    message("-- ${PROJECT_NAME}: making static lib")
    add_library(renderer STATIC ${SOURCE} ${HEADER})
    set(WEBGPU_LINK_TYPE "STATIC")
endif()

set_target_properties(renderer PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

add_subdirectory("${PROJECT_SOURCE_DIR}/vendor/webgpu")

target_link_libraries(renderer PUBLIC webgpu)
target_include_directories(renderer PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_copy_webgpu_binaries(renderer)

function(target_copy_renderer_binaries TARGET)
    target_copy_webgpu_binaries(${TARGET})
    add_custom_command(TARGET ${TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:renderer>
        $<TARGET_FILE_DIR:${TARGET}>
    )
endfunction()

file(GLOB_RECURSE IN_FILES "${PROJECT_SOURCE_DIR}/*.in")
list(FILTER IN_FILES EXCLUDE REGEX "${PROJECT_SOURCE_DIR}/vendor/.*")
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
cmake_minimum_required(VERSION 3.25.0)

project(WINDOW CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

file(GLOB_RECURSE SOURCE 
    "${PROJECT_SOURCE_DIR}/src/*.cpp" 
    "${PROJECT_SOURCE_DIR}/src/*.cc"
    "${PROJECT_SOURCE_DIR}/src/*.hpp"
    "${PROJECT_SOURCE_DIR}/src/*.h"
)
file(GLOB_RECURSE HEADER "${PROJECT_SOURCE_DIR}/include/*.h")

if(WINDOW_LINK_SHARED)
    message("-- ${PROJECT_NAME}: making shared lib")
    add_library(window SHARED ${SOURCE} ${HEADER})
    target_compile_definitions(window PRIVATE ${PROJECT_NAME}_LINK_SHARED)
    set(SDL_SHARED ON)
    set(SDL_STATIC OFF)
    set(BUILD_SHARED_LIBS ON)
    if(WIN32)
        target_compile_definitions(window PRIVATE ${PROJECT_NAME}_LIB_EXPORTS)
    elseif(UNIX)
        target_compile_options(window PRIVATE -fvisibility=hidden)
    endif()
else()
    message("-- ${PROJECT_NAME}: making static lib")
    add_library(window STATIC ${SOURCE} ${HEADER})
    set(SDL_SHARED OFF)
    set(SDL_STATIC ON)
endif()

set_target_properties(window PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
add_subdirectory(vendor/SDL EXCLUDE_FROM_ALL)

target_link_libraries(window PUBLIC SDL3::SDL3)
target_include_directories(window PUBLIC "${PROJECT_SOURCE_DIR}/include/" "${CMAKE_SOURCE_DIR}/utils/")

function(target_copy_window_binaries TARGET)
    if(WINDOW_LINK_SHARED)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:window>
            $<TARGET_FILE_DIR:${TARGET}>
        )
    endif()
    if(SDL_SHARED)
        add_custom_command(TARGET ${TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${TARGET}>
        )
    endif()
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

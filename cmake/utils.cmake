# 定义函数：embed_binary_as_symbol
# 参数：
#   INPUT_FILE     - 输入的二进制文件路径
#   OUTPUT_FILE    - 输出的目标文件路径（可选，默认自动生成）
#   OUTPUT_OBJECT  - 目标文件路径变量（可选出参）
#   SYMBOL_PREFIX  - 自定义符号名前缀（可选，默认自动生成）
function(embed_binary_as_symbol)
    cmake_parse_arguments(
        PARSE_ARGV 0
        "ARG"
        ""
        "INPUT_FILE;OUTPUT_FILE;OUTPUT_OBJECT;SYMBOL_PREFIX"
        ""
    )

    if(NOT ARG_INPUT_FILE)
        message(FATAL_ERROR "INPUT_FILE must be specified!")
    endif()

    string(REGEX REPLACE "[\-./:\\]" "_" SYN_NAME "${ARG_INPUT_FILE}")
    set(SYN_NAME "_binary_${SYN_NAME}")

    if(NOT ARG_SYMBOL_PREFIX)
        set(ARG_SYMBOL_PREFIX ${SYN_NAME})
    else()
        string(REGEX REPLACE "[\-./:\\]" "_" ARG_SYMBOL_PREFIX "${ARG_SYMBOL_PREFIX}")
    endif()

    if(NOT ARG_OUTPUT_FILE)
        set(ARG_OUTPUT_FILE "${ARG_INPUT_FILE}.o")
    endif()

    if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "IA64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
            set(OUT_FORMAT "pe-x86-64")
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "X86")
            set(OUT_FORMAT "pe-i386")
        else()
            message(FATAL_ERROR "Unsupport objcopy output format ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
            set(OUT_FORMAT "elf64-x86-64")
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "i386")
            set(OUT_FORMAT "elf32-i386")
        else()
            message(FATAL_ERROR "Unsupport objcopy output format ${CMAKE_SYSTEM_PROCESSOR}")
        endif()
    endif()

    file(SIZE ${ARG_INPUT_FILE} FILE_SIZE)
    math(EXPR PAD_SIZE "${FILE_SIZE} + 1")
    add_custom_command(
        OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND objcopy
            -I binary -O ${OUT_FORMAT}
            --pad-to=${PAD_SIZE}
            --gap-fill=0x00
            --redefine-sym "${SYN_NAME}_start=${ARG_SYMBOL_PREFIX}_start"
            --redefine-sym "${SYN_NAME}_end=${ARG_SYMBOL_PREFIX}_end"
            --redefine-sym "${SYN_NAME}_size=${ARG_SYMBOL_PREFIX}_size"
            "${ARG_INPUT_FILE}" "${ARG_OUTPUT_FILE}"
        DEPENDS "${ARG_INPUT_FILE}"
        COMMENT "Embedding ${ARG_INPUT_FILE} into ${ARG_OUTPUT_FILE}, symbol ${ARG_SYMBOL_PREFIX}"
    )
    if(ARG_OUTPUT_OBJECT)
        set(${ARG_OUTPUT_OBJECT} ${ARG_OUTPUT_FILE} PARENT_SCOPE)
    endif()
endfunction()
#pragma once

#ifdef _WIN32
    #ifdef LIB_EXPORTS
        #define LIB_API __declspec(dllexport)
    #else
        #define LIB_API __declspec(dllimport)
    #endif
#else
    #define LIB_API __attribute__((visibility("default")))
#endif

LIB_API int test();
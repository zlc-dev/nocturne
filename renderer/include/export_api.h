#ifdef _WIN32
    #ifdef RENDERER_LIB_EXPORTS
        #define RENDERER_LIB_API __declspec(dllexport)
    #else
        #define RENDERER_LIB_API __declspec(dllimport)
    #endif
#else
    #define RENDERER_LIB_API __attribute__((visibility("default")))
#endif

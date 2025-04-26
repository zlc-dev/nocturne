#ifdef _WIN32
    #ifdef WINDOW_LIB_EXPORTS
        #define WINDOW_LIB_API __declspec(dllexport)
    #else
        #define WINDOW_LIB_API __declspec(dllimport)
    #endif
#else
    #define WINDOW_LIB_API __attribute__((visibility("default")))
#endif

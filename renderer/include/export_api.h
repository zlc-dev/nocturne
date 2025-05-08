/*
    define export api
    Copyright (C) 2025 zlc-dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
*/

#if RENDERER_LINK_SHARED
    #ifdef _WIN32
        #ifdef RENDERER_LIB_EXPORTS
            #define RENDERER_LIB_API __declspec(dllexport)
        #else
            #define RENDERER_LIB_API __declspec(dllimport)
        #endif
    #else
        #define RENDERER_LIB_API __attribute__((visibility("default")))
    #endif
#else
    #define RENDERER_LIB_API
#endif

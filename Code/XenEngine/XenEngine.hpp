// Author: Jake Rieger
// Created: 1/7/2025.
//

#pragma once

#if defined(_WIN32) or defined(_WIN64)
    #ifdef XEN_EXPORTS
        #define XEN_API __declspec(dllexport)
    #else
        #define XEN_API __declspec(dllimport)
    #endif
#else
    #define XEN_API
#endif

namespace x {}  // namespace x

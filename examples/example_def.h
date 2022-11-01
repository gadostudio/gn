#pragma once

#include <iostream>
#include <cstdlib>
#include <cassert>

#define EX_THROW_IF(x) \
    if ((x)) { \
        std::cout << #x << "\n"; \
        std::abort(); \
    }

#define EX_THROW_IF_FAILED(x) \
    if (GN_FAILED(x)) { \
        std::cout << "Error: " << #x << "\n"; \
        std::abort(); \
    }

#define EX_ERROR(x) \
    if ((x)) { \
        std::cout << x "\n"; \
        assert(false); \
    }
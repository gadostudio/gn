#pragma once

#include <gn/gn.h>

static constexpr GnBackend g_test_backend =
#if defined(GN_TEST_BACKEND_D3D12)
    GnBackend_D3D12;
#elif defined(GN_TEST_BACKEND_VULKAN)
    GnBackend_Vulkan;
#endif
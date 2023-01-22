#include "../../example_lib.h"

struct HelloWorld : public GnExampleApp
{
    void OnResize(uint32_t width, uint32_t height) override
    {
        GnUpdateSwapchain(swapchain, GnFormat_Unknown, width, height, num_swapchain_buffers, true);
    }
} hello_world;

#pragma once
#include <gn/gn.h>
#include <memory>
#include <vector>
#include <fstream>
#include <optional>
#include "example_def.h"

struct GnExampleWindow
{
    bool open = true;

    virtual ~GnExampleWindow() { }
    virtual bool InitWindow(uint32_t width, uint32_t height) = 0;
    virtual void ProcessEvent() = 0;
    virtual void* GetNativeHandle() const = 0;

    virtual void OnResize(uint32_t width, uint32_t height)
    {
    }
};

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
struct GnExampleWindowWin32 : public GnExampleWindow
{
    HWND hwnd = nullptr;
    bool swapchain_ready = false;
    
    virtual ~GnExampleWindowWin32()
    {
        if (hwnd) DestroyWindow(hwnd);
    }

    bool InitWindow(uint32_t width, uint32_t height) override
    {
        HINSTANCE hinstance = (HINSTANCE)GetModuleHandle(nullptr);

        WNDCLASSEX wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
        wcex.lpfnWndProc = &WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = 0;
        wcex.hInstance = hinstance;
        wcex.hIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
        wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
        wcex.hbrBackground = (HBRUSH)::GetStockObject(BLACK_BRUSH);
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = "gnsamples";
        wcex.hIconSm = wcex.hIcon;

        if (!RegisterClassEx(&wcex)) {
            return false;
        }

        static constexpr DWORD style = WS_OVERLAPPEDWINDOW;// &~(WS_MAXIMIZEBOX | WS_SIZEBOX);

        hwnd = CreateWindowEx(WS_EX_APPWINDOW, "gnsamples", "Gn Samples",
                              style, CW_USEDEFAULT, CW_USEDEFAULT,
                              width, height, nullptr, nullptr,
                              hinstance, this);

        if (!hwnd) return false;

        // Readjust window's client size
        RECT rect{};
        GetWindowRect(hwnd, &rect);
        AdjustWindowRectEx(&rect, style, FALSE, WS_EX_APPWINDOW);
        SetWindowPos(hwnd, nullptr, rect.left, rect.top,
                     rect.right - rect.left,
                     rect.bottom - rect.top, 0);

        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);

        return true;
    }

    void ProcessEvent() override
    {
        MSG msg;

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                open = false;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void* GetNativeHandle() const override
    {
        return (void*)hwnd;
    }

    static LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        if (msg == WM_NCCREATE) {
            LPCREATESTRUCT create_struct = reinterpret_cast<LPCREATESTRUCT>(lparam);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)create_struct->lpCreateParams);
            return DefWindowProc(hwnd, msg, wparam, lparam);
        }

        GnExampleWindowWin32* window = reinterpret_cast<GnExampleWindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

        switch (msg) {
            case WM_CLOSE:
                PostQuitMessage(0);
                return 0;
            case WM_SIZE:
                if (window->swapchain_ready)
                    window->OnResize(LOWORD(lparam), HIWORD(lparam));
                return 0;
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
};
#endif

struct GnExampleApp : private GnExampleWindowWin32
{
    static constexpr uint32_t window_width = 640;
    static constexpr uint32_t window_height = 480;

    static GnExampleApp* g_app;
    GnInstance instance = nullptr;
    GnAdapter adapter = nullptr;
    GnDevice device = nullptr;
    GnSurface surface = nullptr;
    GnQueue queue = nullptr;
    GnSwapchain swapchain = nullptr;

    GnFormat surface_format{};
    uint32_t num_swapchain_buffers = 2;
    int32_t direct_queue_group = -1;
    int32_t present_queue_group = -1;

    GnExampleApp()
    {
        g_app = this;
    }

    virtual ~GnExampleApp()
    {
        GnDeviceWaitIdle(device);
        if (swapchain) GnDestroySwapchain(device, swapchain);
        if (device) GnDestroyDevice(device);
        if (surface) GnDestroySurface(surface);
        if (instance) GnDestroyInstance(instance);
    }

    bool Init()
    {
        GnInstanceDesc instance_desc{};
        instance_desc.backend = GnBackend_Vulkan;

        if (GnCreateInstance(&instance_desc, &instance) != GnSuccess) {
            EX_ERROR("Cannot create instance");
            return false;
        }

        //adapter = GnGetDefaultAdapter(instance);

        GnEnumerateAdapters(
            instance,
            [this](GnAdapter candidate_adapter) {
                GnAdapterProperties properties;
                GnGetAdapterProperties(candidate_adapter, &properties);

                if (properties.type == GnAdapterType_Integrated) {
                    adapter = candidate_adapter;
                }
            });

        if (!InitWindow(window_width, window_height)) {
            EX_ERROR("Cannot initialize window");
            return false;
        }

        GnSurfaceDesc desc{};
        desc.type = GnSurfaceType_Win32;
        desc.hwnd = (HWND)GetNativeHandle();

        if (GnCreateSurface(instance, &desc, &surface) != GnSuccess) {
            EX_ERROR("Cannot create surface");
            return false;
        }

        auto find_queue_group_fn =
            [this](const GnQueueGroupProperties& queue_group) {
                if (direct_queue_group == -1 && queue_group.type == GnQueueType_Direct) {
                    direct_queue_group = queue_group.index;
                }

                if (present_queue_group == -1 && GnIsSurfacePresentationSupported(adapter, queue_group.index, surface)) {
                    present_queue_group = queue_group.index;
                }
            };

        // Find the required queue groups
        GnEnumerateAdapterQueueGroupProperties(adapter, find_queue_group_fn);

        // For now, we assume the driver can do presentation in direct queue
        if (direct_queue_group != present_queue_group) {
            return false;
        }

        GnQueueGroupDesc direct_queue_group_desc{};
        direct_queue_group_desc.index = direct_queue_group;
        direct_queue_group_desc.num_enabled_queues = 1;

        GnDeviceDesc device_desc{};
        device_desc.num_enabled_queue_groups = 1;
        device_desc.queue_group_descs = &direct_queue_group_desc;

        if (GnCreateDevice(adapter, &device_desc, &device) != GnSuccess) {
            EX_ERROR("Cannot create device");
            return false;
        }

        queue = GnGetDeviceQueue(device, direct_queue_group, 0);

        // Find BGRA8Unorm/RGBA8Unorm surface format
        GnEnumerateSurfaceFormats(
            adapter, surface,
            [this](GnFormat format) {
                switch (format) {
                    case GnFormat_BGRA8Unorm:
                    case GnFormat_RGBA8Unorm:
                        surface_format = format;
                        break;
                    default:
                        break;
                }
            });

        GnSwapchainDesc swapchain_desc{};
        swapchain_desc.surface = surface;
        swapchain_desc.usage = GnTextureUsage_ColorTarget;
        swapchain_desc.format = surface_format;
        swapchain_desc.width = window_width;
        swapchain_desc.height = window_height;
        swapchain_desc.num_buffers = num_swapchain_buffers;
        swapchain_desc.vsync = false;

        if (GN_FAILED(GnCreateSwapchain(device, &swapchain_desc, &swapchain))) {
            EX_ERROR("Cannot create swapchain");
            return false;
        }

        swapchain_ready = true;
        OnStart();

        return true;
    }

    int Run()
    {
        while (open) {
            ProcessEvent();
            OnRender();
            GnPresentSwapchain(queue, swapchain);
        }

        return 0;
    }

    virtual void OnStart() { }

    virtual void OnRender() { }
};

static std::optional<std::vector<char>> GnLoadSPIRV(const char* path)
{
    std::vector<char> content;
    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
        return {};

    file.seekg(0, std::ios::end);
    auto length = file.tellg();
    file.seekg(0, std::ios::beg);

    content.resize(length);
    file.read(content.data(), length);

    file.close();
    return content;
}
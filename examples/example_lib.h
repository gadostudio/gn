#pragma once
#include <gn/gn.h>
#include <iostream>
#include <memory>
#include <cassert>

#define EX_THROW_ERROR(x) \
    if ((x)) { \
        std::cout << x "\n"; \
        assert(false); \
    }

struct GnExampleWindow
{
    bool open = true;

    virtual ~GnExampleWindow() { }
    virtual bool Init(uint32_t width, uint32_t height) = 0;
    virtual void ProcessEvent() = 0;
    virtual void* GetNativeHandle() const = 0;
};

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
struct GnExampleWindowWin32 : public GnExampleWindow
{
    HWND hwnd = nullptr;
    
    virtual ~GnExampleWindowWin32()
    {
        if (hwnd) DestroyWindow(hwnd);
    }

    bool Init(uint32_t width, uint32_t height) override
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

        hwnd = CreateWindowEx(WS_EX_APPWINDOW, "gnsamples", "Gn Samples",
                              (WS_OVERLAPPEDWINDOW & ~(WS_MAXIMIZEBOX | WS_SIZEBOX)),
                              CW_USEDEFAULT, CW_USEDEFAULT, width, height,
                              nullptr, nullptr, hinstance, nullptr);

        if (!hwnd) return false;



        ShowWindow(hwnd, SW_SHOW);
        SetForegroundWindow(hwnd);

        return true;
    }

    void ProcessEvent() override
    {
        MSG msg;

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                open = false;
            }

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
        switch (msg) {
            case WM_CLOSE:
                PostQuitMessage(0);
                return 0;
        }

        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
};
#endif

struct GnExampleApp
{
    static GnExampleApp* g_app;
    GnInstance instance = nullptr;
    GnAdapter adapter = nullptr;
    std::unique_ptr<GnExampleWindow> window;

    GnExampleApp() :
        window(std::make_unique<GnExampleWindowWin32>())
    {
        g_app = this;
    }

    ~GnExampleApp()
    {
        if (instance == nullptr)
            GnDestroyInstance(instance);
    }

    bool Init()
    {
        GnInstanceDesc instance_desc{};
        instance_desc.backend = GnBackend_Vulkan;

        if (GnCreateInstance(&instance_desc, &instance) != GnSuccess) {
            EX_THROW_ERROR("Cannot create instance");
            return false;
        }

        adapter = GnGetDefaultAdapter(instance);

        if (!window->Init(640, 480)) {
            EX_THROW_ERROR("Cannot initialize window");
            return false;
        }

        return true;
    }

    int Run()
    {
        while (window->open) {
            window->ProcessEvent();
        }

        return 0;
    }
};
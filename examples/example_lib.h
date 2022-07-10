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
    HWND hWnd;
    
    bool Init()
    {

    }

    void ProcessEvent() override
    {
    }

    void* GetNativeHandle() const override
    {
        return (void*)hWnd;
    }
};
#endif

struct GnExampleApp
{
    static GnExampleApp* g_app;
    GnInstance instance = nullptr;
    GnAdapter adapter = nullptr;

    GnExampleApp()
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
        GnInstanceDesc instance_desc;
        instance_desc.backend = GnBackend_Vulkan;

        if (GnCreateInstance(&instance_desc, nullptr, &instance) != GnSuccess) {
            EX_THROW_ERROR("Cannot create instance");
            return false;
        }

        adapter = GnGetDefaultAdapter(instance);

        return true;
    }

    int Run()
    {
        return 0;
    }
};
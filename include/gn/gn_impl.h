#ifndef GN_IMPL_H_
#define GN_IMPL_H_

#include <gn/gn.h>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

namespace gn
{
    namespace detail
    {
        Expect<Instance*> create_vulkan_instance(const InstanceDesc& desc);

        static void* load_library(const char* name)
        {
#ifdef WIN32
            return (void*)::LoadLibrary(name);
#endif
        }

        template<typename T>
        static T get_library_fn(void* dll_handle, const char* fn_name)
        {
#ifdef WIN32
            return (T)GetProcAddress((HMODULE)dll_handle, fn_name);
#endif
        }
    }

    void AdapterQuery::get(CallbackFn&& callback) const
    {
        for (auto adapter : m_adapters) {
            callback(adapter);
        }
    }

    Expect<Instance*> Instance::create(const InstanceDesc& desc)
    {
        Expect<Instance*> instance;

        switch (desc.backend) {
            case Backend::Vulkan:
                instance = detail::create_vulkan_instance(desc);
                break;
            default:
                return { Error::Unimplemented };
        }

        return instance;
    }
}

#endif // GN_IMPL_H_
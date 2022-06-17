#ifndef GN_IMPL_VULKAN_H_
#define GN_IMPL_VULKAN_H_

namespace gn
{
    struct InstanceVK : public Instance
    {
        InstanceVK(const InstanceDesc& desc) :
            Instance(desc)
        {
        }

        ~InstanceVK()
        {
        }

        void destroy() override final
        {

        }
    };
}

#endif
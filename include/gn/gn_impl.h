#ifndef GN_IMPL_H_
#define GN_IMPL_H_

namespace gn
{
    void AdapterQuery::get(CallbackFn&& callback) const
    {
        for (auto adapter : m_adapters) {
            callback(adapter);
        }
    }

    Expect<Instance*> Instance::create(const InstanceDesc& desc)
    {
        return { Error::Unimplemented };
    }
}

#endif // GN_IMPL_H_
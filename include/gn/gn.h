#ifndef GN_H_
#define GN_H_

#include <vector>
#include <bitset>
#include <functional>
#include <cstdint>
#include <cassert>

#include "gn_detail.h"

namespace gn
{
    class AdapterQuery;
    class Instance;
    class Adapter;
    class Device;

    enum class Error
    {
        Unknown,
        Unimplemented,
        OutOfMemory,
    };

    enum class Backend
    {
        Auto,
        D3D11,
        D3D12,
        Vulkan
    };

    enum class AdapterType
    {
        Discrete,
        Integrated,
        Software,

        Count
    };

    enum class VendorID
    {
        
    };

    enum class Feature
    {
        FullDrawIndexRange32Bit,
        TextureCubeArray,
        GeometryShader,
        TessellationShader,
        NativeMultiDrawIndirect,

        Count
    };

    struct InstanceDesc
    {
        Backend backend                 = Backend::Auto;
        bool enable_debugging           = false;
        bool enable_validation          = false;
        bool enable_backend_validation  = false;
    };

    template<typename T, std::enable_if_t<!std::is_same_v<T, Error>, bool> = true>
    class Expect
    {
    public:
        Expect(T&& value) noexcept :
            m_has_value(true),
            m_value(std::move(value))
        {
        }

        template<typename... Args>
        Expect(bool, Args&&... args) :
            m_has_value(true),
            m_value(std::forward<Args>(args)...)
        {
        }

        Expect(Error error) noexcept :
            m_has_value(false),
            m_error(error)
        {
            assert(true);
        }

        const T* operator->() const noexcept { return &m_value; }

        T* operator->() noexcept { return &m_value; }

        const T& operator*() const& noexcept { return m_value; }

        T& operator*() & noexcept { return m_value; }

        const T&& operator*() const&& noexcept { return std::move(m_value); }

        T&& operator*() && noexcept { return std::move(m_value); }

        explicit operator bool() const noexcept { return m_has_value; }

        bool has_value() const noexcept { return m_has_value; }
        
        T& value() & noexcept
        {
            GN_CHECK(m_has_value && "Expected value, not error");
            return m_value;
        }
        
        const T& value() const& noexcept
        {
            GN_CHECK(m_has_value && "Expected value, not error");
            return m_value;
        }

        T&& value() && noexcept
        {
            GN_CHECK(m_has_value && "Expected value, not error");
            return std::move(m_value);
        }
        
        const T&& value() const&& noexcept
        {
            GN_CHECK(m_has_value && "Expected value, not error");
            return std::move(m_value);
        }

        Error error() const noexcept { return m_has_value ? Error::Unknown : m_error; }

    private:
        bool m_has_value;
        
        union
        {
            T m_value;
            Error m_error;
        };
    };

    class AdapterQuery
    {
    public:
        using CallbackFn = std::function<void(Adapter*)>;
        using CustomConditionFn = std::function<bool(Adapter*)>;

        AdapterQuery(const std::vector<Adapter*>& adapters) :
            m_adapters(adapters)
        {
        }

        AdapterQuery& with_type(AdapterType type)
        {
            size_t bit_idx = static_cast<size_t>(type);
            m_types.set(bit_idx, true);
            return *this;
        }

        AdapterQuery& with_vendor_id(VendorID vendor_id)
        {
            return *this;
        }

        AdapterQuery& has_feature(Feature feature)
        {
            size_t bit_idx = static_cast<size_t>(feature);
            m_features.set(bit_idx, true);
            return *this;
        }

        template<typename... Args, std::enable_if_t<std::conjunction_v<std::is_same<Feature, Args>...>, bool> = true>
        AdapterQuery& has_features(Feature feature, Args... features)
        {
            has_feature(feature);
            (has_feature(features), ...);
            return *this;
        }

        AdapterQuery& has_geometry_stage()
        {
            return has_feature(Feature::GeometryShader);
        }

        AdapterQuery& has_tessellation_stage()
        {
            return has_feature(Feature::TessellationShader);
        }

        AdapterQuery& custom_condition(CustomConditionFn condition)
        {
            m_custom_condition_fn = condition;
            return *this;
        }

        void get(CallbackFn callback) const;

        std::vector<Adapter*> get() const
        {
            std::vector<Adapter*> result;

            get([&result](Adapter* adapter) {
                    result.push_back(adapter);
                });

            return std::move(result);
        }

    private:
        const std::vector<Adapter*>& m_adapters;
        std::bitset<static_cast<size_t>(AdapterType::Count)> m_types;
        std::bitset<static_cast<size_t>(Feature::Count)> m_features;
        CustomConditionFn m_custom_condition_fn;
    };

    class Instance
    {
    public:
        struct Builder
        {
            InstanceDesc desc{};

            Builder() { }

            Builder& set_backend(Backend backend)
            {
                desc.backend = backend;
                return *this;
            }

            Builder& enable_debugging(bool should_enable = true)
            {
                desc.enable_debugging = should_enable;
                return *this;
            }

            Builder& enable_validation(bool should_enable = true)
            {
                desc.enable_validation = should_enable;
                return *this;
            }

            Builder& enable_backend_validation(bool should_enable = true)
            {
                desc.enable_backend_validation = should_enable;
                return *this;
            }

            Expect<Instance*> create()
            {
                return Instance::create(desc);
            }
        };

        Instance(const InstanceDesc& desc) :
            m_desc(desc)
        {
        }

        virtual ~Instance() { }

        Adapter* default_adapter() const noexcept
        {
            return m_adapters[0];
        }

        uint32_t num_adapters() const noexcept
        {
            return (uint32_t)m_adapters.size();
        }

        const std::vector<Adapter*>& get_adapters() const
        {
            return m_adapters;
        }

        void get_adapters(uint32_t num_adapters, Adapter** adapters)
        {
            if (num_adapters > 0) {
                num_adapters = num_adapters > (uint32_t)m_adapters.size() ?
                    (uint32_t)m_adapters.size() : num_adapters;

                for (uint32_t i = 0; i < num_adapters; i++) {
                    adapters[i] = m_adapters[i];
                }
            }
        }

        AdapterQuery query_adapters() const noexcept
        {
            return AdapterQuery(m_adapters);
        }

        const Backend backend() const noexcept
        {
            return m_desc.backend;
        }

        virtual void destroy() = 0;

        static Expect<Instance*> create(const InstanceDesc& desc);

    protected:
        InstanceDesc m_desc;
        std::vector<Adapter*> m_adapters;
    };

    class Adapter
    {
    public:
        Adapter()
        {
        }
    };

    class Device
    {
    public:
        virtual void destroy() = 0;
    };
}

#ifdef GN_IMPLEMENTATION
#include <gn/gn_impl.h>
#include <gn/gn_impl_vulkan.h>
#endif

#endif // GN_H_
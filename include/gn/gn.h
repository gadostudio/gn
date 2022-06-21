#ifndef GN_H_
#define GN_H_

#include <vector>
#include <bitset>
#include <functional>

#include "gn_detail.h"

namespace gn
{
    class AdapterQuery;
    class Instance;
    class Adapter;
    class Device;
    class Buffer;
    class Texture;
    class ResourceTable;
    class CommandBuffer;
    class CommandBundle;

    enum class Error
    {
        Unknown,
        Unimplemented,
        BackendNotAvailable,
        InitializationFailed,
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
        Unknown,
        Discrete,
        Integrated,
        Software,

        Count
    };

    enum class VendorID
    {
        Unknown,
        Count
    };

    enum class Limit
    {
        Count
    };

    enum class Feature
    {
        FullDrawIndexRange32Bit,
        TextureCubeArray,
        GeometryShader,
        TessellationShader,
        NativeMultiDrawIndirect,
        DrawIndirectFirstInstance,

        Count
    };

    enum class PipelineBindPoint
    {
        Graphics,
        Compute,

        Count
    };

    struct InstanceDesc
    {
        Backend backend;
        bool    enable_debugging;
        bool    enable_validation;
        bool    enable_backend_validation;
    };

    struct AdapterProperties
    {
        char        device_name[GN_MAX_CHARS];
        uint32_t    vendor_id;
        AdapterType adapter_type;
    };

    struct Rect2D
    {
        uint32_t x;
        uint32_t y;
        uint32_t width;
        uint32_t height;
    };

    struct Viewport
    {
        float x;
        float y;
        float width;
        float height;
        float min_depth;
        float max_depth;
    };

    template<typename T, std::enable_if_t<!std::is_same_v<T, Error>, bool> = true>
    class Expect
    {
    public:
        using Self = Expect<T>;

        Expect() noexcept :
            m_state(State::Empty),
            m_error(Error::Unknown)
        {
        }

        Expect(T&& value) noexcept :
            m_state(State::HasValue),
            m_value(std::move(value))
        {
        }

        template<typename... Args>
        Expect(bool, Args&&... args) :
            m_state(State::HasValue),
            m_value(std::forward<Args>(args)...)
        {
        }

        Expect(Error error) noexcept :
            m_state(State::HasError),
            m_error(error)
        {
            assert(true);
        }

        ~Expect()
        {
            if (has_value() && destructible) {
                m_value.~T();
            }
        }

        template<std::enable_if_t<std::is_copy_assignable_v<T>, bool> = true>
        Self& operator=(const Expect<T>& other)
        {
            switch (other.m_state) {
                case State::HasValue:
                    m_value = other.m_value;
                    break;
                case State::HasError:
                    m_error = other.m_error;
                    break;
                default:
                    break;
            }

            m_state = other.m_state;

            return *this;
        }

        const T* operator->() const noexcept { return &m_value; }

        T* operator->() noexcept { return &m_value; }

        const T& operator*() const& noexcept { return m_value; }

        T& operator*() & noexcept { return m_value; }

        const T&& operator*() const&& noexcept { return std::move(m_value); }

        T&& operator*() && noexcept { return std::move(m_value); }

        explicit operator bool() const noexcept { return has_value(); }

        template<typename... Args>
        void emplace(Args&&... args)
        {
            if (has_value() && destructible) {
                m_value.~T();
            }

            if constexpr (std::is_trivially_constructible_v<T>) {
                new(&m_value) T{ std::forward<Args>(args)... };
            }
            else {
                new(&m_value) T(std::forward<Args>(args)...);
            }

            m_state = State::HasValue;
        }

        void emplace_error(Error error)
        {
            if (has_value() && destructible) {
                m_value.~T();
            }

            m_error = error;
            m_state = State::HasError;
        }

        bool has_value() const noexcept { return m_state == State::HasValue; }
        
        T& value() & noexcept
        {
            GN_CHECK(has_value() && "Expected value");
            return m_value;
        }
        
        const T& value() const& noexcept
        {
            GN_CHECK(has_value() && "Expected value");
            return m_value;
        }

        T&& value() && noexcept
        {
            GN_CHECK(has_value() && "Expected value");
            return std::move(m_value);
        }
        
        const T&& value() const&& noexcept
        {
            GN_CHECK(has_value() && "Expected value");
            return std::move(m_value);
        }

        Error error() const noexcept { return m_state != State::HasError ? Error::Unknown : m_error; }

    private:
        static constexpr bool destructible =
            std::is_destructible_v<T> || std::is_trivially_destructible_v<T>;

        enum class State
        {
            Empty,
            HasValue,
            HasError
        };

        State m_state;
        
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

        AdapterQuery& custom_condition(CustomConditionFn&& condition)
        {
            m_custom_condition_fn = condition;
            return *this;
        }

        void get(CallbackFn&& callback) const;

        std::vector<Adapter*> get() const
        {
            std::vector<Adapter*> result;
            get([&result](Adapter* adapter) { result.push_back(adapter); });
            return result;
        }

    private:
        const std::vector<Adapter*>& m_adapters;
        std::bitset<detail::enum_count<AdapterType>> m_types;
        std::bitset<detail::enum_count<Feature>> m_features;
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
        template<typename T>
        auto get_limit(Limit limit) const -> T
        {
            if (limit >= Limit::Count) {
                return 0;
            }

            const auto& value = m_limits[static_cast<size_t>(limit)];
            T ret_val = 0;
            
            if (std::holds_alternative<uint32_t>(value)) {
                ret_val = static_cast<T>(std::get<uint32_t>(value));
            }
            else if (std::holds_alternative<float>(value)) {
                ret_val = static_cast<T>(std::get<float>(value));
            }

            return ret_val;
        }

        bool is_feature_present(Feature feature) const
        {
            if (feature >= Feature::Count) {
                return false;
            }

            return m_features[static_cast<size_t>(feature)];
        }

        bool is_format_present()
        {
            return false;
        }

    protected:
        AdapterProperties m_properties{};
        std::bitset<detail::enum_count<Feature>> m_features;
        detail::LimitMap<detail::enum_count<Limit>> m_limits;
    };

    class Device
    {
    public:
        virtual void create_buffer() = 0;
        virtual void create_texture() = 0;
        virtual void create_resource_table_layout() = 0;
        virtual void create_resource_table_pool() = 0;
        virtual void create_pipeline_layout() = 0;
        virtual void create_graphics_pipeline() = 0;
        virtual void create_compute_pipeline() = 0;
        virtual void create_command_pool() = 0;
        virtual void create_command_buffers() = 0;
        virtual void create_command_bundles() = 0;

        virtual void destroy() = 0;
    };

    class Buffer
    {
    public:
        virtual void destroy() = 0;
    };

    class ComputeCommandEncoder
    {
    public:
        ComputeCommandEncoder() {}

        void dispatch(uint32_t thread_group_x, uint32_t thread_group_y, uint32_t thread_group_z)
        {

        }

        void dispatch_indirect(Buffer* indirect_buffer, uint64_t offset = 0)
        {

        }

        void finish();
    };

    class CopyCommandEncoder
    {
    public:
        CopyCommandEncoder() {}
    };

    class CommandBuffer
    {
    public:
        CommandBuffer();

        Error begin_rendering()
        {
            m_is_rendering = true;
            return Error::Unimplemented;
        }

        void set_graphics_pipeline() noexcept;

        void set_graphics_resource_table(uint32_t slot) noexcept;

        void set_graphics_resource_tables(uint32_t first_slot, uint32_t num_resource_tables) noexcept;

        template<typename T>
        void set_graphics_shader_constant(uint32_t element_offset, T constant) noexcept
        {

        }

        template<typename T, size_t Size>
        void set_graphics_shader_constants(uint32_t element_offset, const T(&values)[Size]) noexcept
        {
            set_shader_constants_raw(element_offset * sizeof(T), sizeof(values), values);
        }

        void set_graphics_shader_constants_raw(uint32_t byte_offset, uint32_t size, const void* values) noexcept
        {
        }

        void set_index_buffer(Buffer* index_buffer, uint64_t offset = 0) noexcept
        {

        }

        void set_vertex_buffer(uint32_t slot, Buffer* index_buffer, uint64_t offset = 0) noexcept
        {

        }

        void set_vertex_buffers(uint32_t first_slot,
                                uint32_t num_buffers,
                                const Buffer** vertex_buffer,
                                const uint64_t* offsets = nullptr) noexcept
        {

        }

        inline void set_viewport(uint32_t slot,
                                 float x,
                                 float y,
                                 float width,
                                 float height,
                                 float min_depth,
                                 float max_depth) noexcept
        {
            GN_DBG_ASSERT(slot < 16 && "Slot out of range");

            Viewport& viewport = m_graphics_state_block.viewports[slot];
            viewport.x = x;
            viewport.y = y;
            viewport.width = width;
            viewport.height = height;
            viewport.min_depth = min_depth;
            viewport.max_depth = max_depth;

            m_graphics_state_block.viewports_update_flags |= 1 << slot;
        }

        inline void set_viewport(uint32_t slot, const Viewport& viewport)
        {
            GN_DBG_ASSERT(slot < 16 && "Slot out of range");
            m_graphics_state_block.viewports[slot] = viewport;
            m_graphics_state_block.viewports_update_flags |= 1 << slot;
        }

        template<uint32_t NumViewports>
        inline void set_viewports(uint32_t first_slot, const Viewport(&viewports)[NumViewports]) noexcept
        {
            set_viewports(first_slot, NumViewports, viewports);
        }

        void set_viewports(uint32_t first_slot,
                           uint32_t num_viewports,
                           const Viewport* viewports) noexcept
        {
            GN_DBG_ASSERT(first_slot < 16 && "Slot out of range");
            GN_DBG_ASSERT(num_viewports + first_slot < 16 && "Slot out of range");

            for (uint32_t i = 0; i < num_viewports; i++) {
                m_graphics_state_block.viewports[i + first_slot] = viewports[i];
            }

            // Notify viewport updates
            m_graphics_state_block.viewports_update_flags |=
                ((1 << (num_viewports + first_slot)) - 1) & ~((1 << first_slot) - 1);
        }

        inline void set_scissor(uint32_t slot,
                                uint32_t x,
                                uint32_t y,
                                uint32_t width,
                                uint32_t height) noexcept
        {
            GN_DBG_ASSERT(slot < 16 && "Slot out of range");

            Rect2D& scissor = m_graphics_state_block.scissors[slot];
            scissor.x = x;
            scissor.y = y;
            scissor.width = width;
            scissor.height = height;

            m_graphics_state_block.scissors_update_flags |= 1 << slot;
        }

        inline void set_scissor(uint32_t slot, const Rect2D& scissor) noexcept
        {
            GN_DBG_ASSERT(slot < 16 && "Slot out of range");
            m_graphics_state_block.scissors[slot] = scissor;
            m_graphics_state_block.scissors_update_flags |= 1 << slot;
        }

        template<uint32_t NumScissors>
        inline void set_scissors(uint32_t first_slot, const Rect2D (&scissors)[NumScissors]) noexcept
        {
            set_viewports(first_slot, NumScissors, scissors);
        }

        void set_scissors(uint32_t first_slot, uint32_t num_scissors, const Rect2D* scissors) noexcept
        {
            GN_DBG_ASSERT(first_slot < 16 && "Slot out of range");
            GN_DBG_ASSERT(num_scissors + first_slot < 16 && "Slot out of range");

            for (uint32_t i = 0; i < num_scissors; i++) {
                m_graphics_state_block.scissors[i + first_slot] = scissors[i];
            }

            // Notify viewport updates
            m_graphics_state_block.scissors_update_flags |=
                ((1 << (num_scissors + first_slot)) - 1) & ~((1 << first_slot) - 1);
        }

        inline void set_blend_constants(const float blend_constants[4]) noexcept
        {
            m_graphics_state_block.blend_constants[0] = blend_constants[0];
            m_graphics_state_block.blend_constants[1] = blend_constants[1];
            m_graphics_state_block.blend_constants[2] = blend_constants[2];
            m_graphics_state_block.blend_constants[3] = blend_constants[3];
            m_graphics_state_block.blend_constants_update = true;
        }

        inline void set_blend_constants(float r, float g, float b, float a) noexcept
        {
            m_graphics_state_block.blend_constants[0] = r;
            m_graphics_state_block.blend_constants[1] = g;
            m_graphics_state_block.blend_constants[2] = b;
            m_graphics_state_block.blend_constants[3] = a;
            m_graphics_state_block.blend_constants_update = true;
        }

        inline void set_stencil_ref(uint32_t ref) noexcept
        {
            m_graphics_state_block.stencil_ref = ref;
            m_graphics_state_block.stencil_ref_update = true;
        }

        inline void draw(uint32_t num_vertices, uint32_t first_vertex) noexcept
        {
            draw_instanced(num_vertices, 1, first_vertex, 0);
        }

        void draw_instanced(uint32_t num_vertices,
                            uint32_t num_instances,
                            uint32_t first_vertex,
                            uint32_t first_instances) noexcept
        {
            if (!m_is_rendering) {
                return;
            }
            // TODO: Apply state changes and draw
        }

        inline void draw_indexed(uint32_t num_indices,
                                 uint32_t first_index,
                                 int32_t base_vertex) noexcept
        {
            draw_indexed_instanced(num_indices, 1, first_index, base_vertex, 0);
        }

        inline void draw_indexed_instanced(uint32_t num_indices,
                                           uint32_t num_instances,
                                           uint32_t first_index,
                                           int32_t base_vertex,
                                           uint32_t first_instances) noexcept
        {
            if (!m_is_rendering) {
                return;
            }

            // TODO: Apply state changes and draw

        }

        inline void draw_instanced_indirect(uint32_t draw_count,
                                            uint32_t stride,
                                            Buffer* indirect_buffer,
                                            uint64_t offset = 0) noexcept
        {
            // TODO: Apply state changes and draw

        }

        inline void draw_indexed_instanced_indirect(uint32_t draw_count,
                                                    uint32_t stride,
                                                    Buffer* indirect_buffer,
                                                    uint64_t offset = 0) noexcept
        {
            // TODO: Apply state changes and draw

        }

        void execute_bundles(uint32_t num_bundles);

        void finish_rendering()
        {
            m_is_rendering = false;
        }

    private:
        struct GraphicsStateBlock
        {
            // Viewport state block
            Viewport    viewports[16];
            uint32_t    viewports_update_flags;
            Rect2D      scissors[16];
            uint32_t    scissors_update_flags;
        
            // Fragment output stage state block
            float       blend_constants[4];
            bool        blend_constants_update;
            uint32_t    stencil_ref;
            bool        stencil_ref_update;
        };
        
        CommandBuffer*      m_parent_cmd_buffer;
        GraphicsStateBlock  m_graphics_state_block{};
        bool                m_graphics_state_changed;
        bool                m_is_rendering;
    };

    class CommandBundle
    {
    public:
        CommandBundle() {}


    };
}

#endif // GN_H_
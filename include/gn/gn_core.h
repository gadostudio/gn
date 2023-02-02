#ifndef GN_CORE_H_
#define GN_CORE_H_

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <bitset>
#include <optional>
#include <algorithm>
#include <new>
#include <mutex>
#include <shared_mutex>
#include <functional>

#if defined(_MSC_VER)
#define GN_COMPILER_MSVC
#elif defined(__clang__)
#define GN_COMPILER_CLANG
#elif defined(__GNUC__)
#define GN_COMPILER_GCC
#endif

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_ASSERT(x) assert(x)
#define GN_CHECK(x) GN_DBG_ASSERT(x)

#ifdef GN_COMPILER_MSVC
#define GN_SAFEBUFFERS __declspec(safebuffers)
#define GN_COMPILER_UNREACHABLE __assume(0)
#else
#define GN_SAFEBUFFERS
#define GN_COMPILER_UNREACHABLE
#endif

#if __has_cpp_attribute(unlikely)
#define GN_UNLIKELY [[unlikely]]
#else
#define GN_UNLIKELY
#endif

#ifdef NDEBUG
#define GN_UNREACHABLE() GN_COMPILER_UNREACHABLE
#else
#define GN_UNREACHABLE() GN_ASSERT(false && "Unreachable");
#endif

#if defined(_WIN32)
#include <unknwn.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

#ifdef _WIN32
#include <malloc.h>
#include <crtdbg.h>
#define _CRTDBG_MAP_ALLOC
#define GN_ALLOCA(size) _alloca(size)
#else
#define GN_ALLOCA(size) alloca(size)
#endif

#undef min
#undef max

template<typename T, typename T2>
static constexpr T GnMax(T a, T2 b) noexcept
{
    return (a > b) ? a : b;
}

template<typename T, typename T2, typename... Rest>
static constexpr T GnMax(T a, T2 b, Rest... rest) noexcept
{
    return (a > b) ? a : GnMax<T2, Rest...>(b, rest...);
}

template<typename T, typename T2>
static constexpr T GnMin(T a, T2 b) noexcept
{
    return (a < b) ? a : b;
}

template<typename T, typename... Args>
inline static constexpr bool GnContainsBit(T op, Args... args) noexcept
{
    T mask = (args | ...);
    return (op & mask) == mask;
}

template<typename T, typename... Args>
inline static constexpr bool GnHasBit(T op, Args... args) noexcept
{
    T mask = (args | ...);
    return (op & mask) != 0;
}

template<typename T>
inline static T* GnAllocate(std::size_t count = 1, std::size_t align = alignof(T))
{
    return (T*)::operator new[](sizeof(T) * count, std::align_val_t{ align }, std::nothrow);
}

template<typename T, std::enable_if_t<std::is_pointer_v<T>, bool> = true>
inline static void GnFree(T ptr, std::size_t align = alignof(std::remove_pointer_t<T>))
{
    ::operator delete[](ptr, std::align_val_t{ align }, std::nothrow);
}

template<typename T>
inline static size_t GnCalcHash(const T& data)
{
    return std::hash<T>{}(data); // TODO: Replace with a better one perhaps?
}

template<typename T>
inline static void GnCombineHash(size_t& hash, const T& data)
{
    if constexpr (sizeof(size_t) == 8) {
        hash = GnCalcHash(data) + 0x9e3779b97f4a7c16 + (hash << 6) + (hash >> 2);
    }
    else if constexpr (sizeof(size_t) == 4) {
        hash = GnCalcHash(data) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
    }
}

template<typename T, typename... Args>
inline static void GnCombineHash(size_t& hash, const T& data, const Args&... args)
{
    GnCombineHash(hash, data);
    (GnCombineHash(hash, args), ...);
}

struct GnPoolHeader
{
    GnPoolHeader* next_pool;
};

struct GnPoolChunk
{
    GnPoolChunk* next_chunk;
};

// Growable pool
template<typename T>
struct GnPool
{
    static constexpr std::size_t alloc_size = GnMax(sizeof(T), sizeof(GnPoolHeader));
    static constexpr std::size_t alloc_alignment = GnMax(alignof(T), alignof(GnPoolHeader));

    GnPoolHeader* first_pool = nullptr;
    GnPoolHeader* current_pool = nullptr;
    GnPoolChunk* current_allocation = nullptr;
    GnPoolChunk* last_allocation = nullptr;
    std::size_t objects_per_block;
    std::size_t num_reserved = 0;
    std::size_t num_allocated = 0;
    bool reserve_once = false;
    std::mutex mutex;

    GnPool(std::size_t objects_per_block, bool reserve_once = false) noexcept :
        objects_per_block(objects_per_block)
    {
    }

    ~GnPool()
    {
        GnPoolHeader* pool = first_pool;
        while (pool != nullptr) {
            GnPoolHeader* next_pool = pool->next_pool;
            ::operator delete(pool, std::align_val_t{ alloc_alignment }, std::nothrow);
            pool = next_pool;
        }
    }

    void* allocate() noexcept
    {
        std::scoped_lock lock(mutex);

        if (num_allocated >= num_reserved) {
            if (!_reserve_new_block()) {
                return nullptr;
            }
        }

        GnPoolChunk* free_chunk = current_allocation;
        //last_allocation = current_allocation;
        current_allocation = current_allocation->next_chunk;
        ++num_allocated;

        return free_chunk;
    }

    void free(void* ptr) noexcept
    {
        std::scoped_lock lock(mutex);

        std::memset(ptr, 0, alloc_size);
        GnPoolChunk* chunk = reinterpret_cast<GnPoolChunk*>(ptr);
        chunk->next_chunk = current_allocation;
        current_allocation = chunk;
        --num_allocated;
    }

    bool _reserve_new_block() noexcept
    {
        if (reserve_once && num_reserved == objects_per_block) return false;

        // Allocate the pool
        // One extra storage is required for the pool header. It may waste space a bit if the object is too large.
        std::size_t size = alloc_size * (objects_per_block + 1);
        uint8_t* pool = (uint8_t*)::operator new[](size, std::align_val_t{ alloc_alignment }, std::nothrow);
        
        if (pool == nullptr)
            return false;

        std::memset(pool, 0, size);

        // Initialize the pool header
        GnPoolHeader* pool_header = reinterpret_cast<GnPoolHeader*>(pool);

        if (current_pool)
            current_pool->next_pool = pool_header;
        else
            first_pool = pool_header;

        current_pool = pool_header;
        pool_header->next_pool = nullptr;

        // Initialize free list.
        GnPoolChunk* current_chunk = reinterpret_cast<GnPoolChunk*>(pool + alloc_size);
        current_allocation = current_chunk;

        for (std::size_t i = 1; i < objects_per_block; i++) {
            current_chunk->next_chunk = reinterpret_cast<GnPoolChunk*>(reinterpret_cast<uint8_t*>(current_chunk) + alloc_size);
            current_chunk = current_chunk->next_chunk;
        }

        num_reserved += objects_per_block;

        return true;
    }
};

template<typename T, std::enable_if_t<std::is_pod_v<T>, bool> = true>
struct GnVector
{
    T* first_ptr = nullptr;
    T* last_ptr = nullptr;
    T* end_ptr = nullptr;

    GnVector() noexcept
    {
    }

    GnVector(GnVector&& other) noexcept :
        first_ptr(other.first_ptr),
        last_ptr(other.last_ptr),
        end_ptr(other.end_ptr)
    {
        other.first_ptr = nullptr;
        other.last_ptr = nullptr;
        other.end_ptr = nullptr;
    }

    ~GnVector()
    {
        if (first_ptr)
            ::operator delete[](first_ptr, std::align_val_t{ alignof(T) }, std::nothrow);
    }

    inline T& operator[](size_t n) noexcept
    {
        return first_ptr[n];
    }

    inline const T& operator[](size_t n) const noexcept
    {
        return first_ptr[n];
    }

    inline T* data() noexcept
    {
        return first_ptr;
    }

    inline const T* data() const noexcept
    {
        return first_ptr;
    }

    bool push_back(const T& value) noexcept
    {
        if (last_ptr == end_ptr) {
            size_t last_capacity = capacity();
            if (!reserve(1 + last_capacity + last_capacity / 2))
                return false;
        }

        new(last_ptr++) T(value);
        return true;
    }

    template<typename... Args>
    std::optional<std::reference_wrapper<T>> emplace_back(Args&&... args) noexcept
    {
        if (last_ptr == end_ptr) {
            size_t last_capacity = capacity();
            if (!reserve(1 + last_capacity + last_capacity / 2))
                return false;
        }

        auto ptr = new(last_ptr++) T{ std::forward<Args>(args)... };
        return { std::ref(*ptr) };
    }

    bool resize(size_t n) noexcept
    {
        if (n >= size()) {
            if (!reserve(n))
                return false;
        }

        last_ptr = first_ptr + n;
        return true;
    }

    bool reserve(size_t n) noexcept
    {
        if (n <= capacity())
            return true;

        T* new_storage = (T*)::operator new[](n * sizeof(T), std::align_val_t{ alignof(T) }, std::nothrow);

        if (new_storage == nullptr)
            return false;

        size_t last_size = size();

        if (first_ptr) {
            std::memcpy(new_storage, first_ptr, last_size * sizeof(T));
            ::operator delete[](first_ptr, std::align_val_t{ alignof(T) }, std::nothrow);
        }

        first_ptr = new_storage;
        last_ptr = new_storage + last_size;
        end_ptr = new_storage + n;

        return true;
    }

    size_t size() const noexcept
    {
        return last_ptr - first_ptr;
    }

    size_t capacity() const noexcept
    {
        return end_ptr - first_ptr;
    }
};

// Only use this for POD structs!!
template<typename T, size_t Size, std::enable_if_t<std::is_trivial_v<T>, bool> = true>
struct GnSmallVector
{
    T local_storage[Size]{};
    T* storage = nullptr;
    size_t size = 0;
    size_t capacity = Size;

    inline GnSmallVector() noexcept : storage(local_storage)
    {
    }

    ~GnSmallVector()
    {
        if (storage != local_storage)
            ::operator delete[](storage, std::align_val_t{alignof(T)}, std::nothrow);
    }

    GnSmallVector& operator=(const GnSmallVector& other)
    {
        if (other.storage != other.local_storage)
            reserve(other.size);

        std::memcpy(storage, other.storage, other.size * sizeof(T));
        size = other.size;
        capacity = other.capacity;
        return *this;
    }

    GnSmallVector& operator=(GnSmallVector&& other)
    {
        if (other.storage == other.local_storage)
            std::memcpy(storage, other.storage, other.size * sizeof(T));
        else
            storage = other.storage;

        size = other.size;
        capacity = other.capacity;
        std::memset(other.storage, 0, other.size * sizeof(T));
        other.size = 0;
        other.capacity = Size;
        return *this;
    }

    inline T& operator[](size_t n) noexcept
    {
        return storage[n];
    }

    inline const T& operator[](size_t n) const noexcept
    {
        return storage[n];
    }

    inline bool push_back(const T& value) noexcept
    {
        if (size == capacity)
            if (!reserve(capacity + (capacity / 2)))
                return false;

        storage[size++] = value;
        return true;
    }

    template<typename... Args>
    inline std::optional<std::reference_wrapper<T>> emplace_back(Args&&... args) noexcept
    {
        if (size == capacity)
            if (!reserve(capacity + (capacity / 2)))
                return {};

        auto ptr = new(storage + size++) T{ std::forward<Args>(args)... };

        return { std::ref(*ptr) };
    }

    template<typename... Args>
    inline T* emplace_back_ptr(Args&&... args) noexcept
    {
        if (size == capacity)
            if (!reserve(capacity + (capacity / 2)))
                return nullptr;

        auto ptr = new(storage + size++) T{ std::forward<Args>(args)... };

        return ptr;
    }

    inline bool resize(size_t n) noexcept
    {
        size = n;
        return reserve(n);
    }

    bool reserve(size_t n) noexcept
    {
        if (n > capacity) {
            T* new_storage = (T*)::operator new[](n * sizeof(T), std::align_val_t{ alignof(T) }, std::nothrow);

            if (new_storage == nullptr)
                return false;

            std::memset(new_storage, 0, n * sizeof(T));
            std::memcpy(new_storage, storage, size * sizeof(T));

            if (storage != local_storage)
                ::operator delete[](storage, std::align_val_t{alignof(T)}, std::nothrow);

            storage = new_storage;
            capacity = n;
        }

        return true;
    }
};

template<typename T, size_t Size, std::enable_if_t<std::is_pod_v<T>, bool> = true>
struct GnSmallQueue
{
    T local_storage[Size]{};
    T* data = nullptr;
    T* read_ptr = nullptr;
    T* write_ptr = nullptr;
    size_t capacity = Size;

    GnSmallQueue() :
        data(local_storage),
        read_ptr(data),
        write_ptr(data)
    {
    }

    ~GnSmallQueue()
    {
        if (data != local_storage)
            ::operator delete[](data, std::align_val_t{ alignof(T) }, std::nothrow);
    }

    inline bool push(const T& value) noexcept
    {
        if (num_items_written() == capacity)
            if (!reserve(capacity + (capacity / 2)))
                return false;

        *write_ptr = value;
        write_ptr++;

        return true;
    }

    template<typename... Args>
    std::optional<std::reference_wrapper<T>> emplace(Args&&... args) noexcept
    {
        if (num_items_written() == capacity)
            if (!reserve(capacity + (capacity / 2)))
                return {};

        auto ptr = new(write_ptr++) T{ std::forward<Args>(args)... };

        return { std::ref(*ptr) };
    }

    inline T* pop() noexcept
    {
        if (write_ptr == read_ptr)
            return nullptr;

        return read_ptr++;
    }

    inline T* pop_all() noexcept
    {
        T* tmp = read_ptr;
        read_ptr = write_ptr;
        return tmp;
    }

    inline size_t size() const noexcept
    {
        return write_ptr - read_ptr;
    }

    inline size_t num_items_written() const noexcept
    {
        return write_ptr - data;
    }

    inline size_t num_items_read() const noexcept
    {
        return read_ptr - data;
    }

    inline bool empty() const noexcept
    {
        return size() == 0;
    }

    inline void clear() noexcept
    {
        write_ptr = data;
        read_ptr = data;
    }

    bool reserve(size_t n) noexcept
    {
        if (n > capacity) {
            T* new_storage = (T*)::operator new[](n * sizeof(T), std::align_val_t{ alignof(T) }, std::nothrow);

            if (new_storage == nullptr)
                return false;

            std::memcpy(new_storage, data, num_items_written() * sizeof(T));

            if (data != local_storage)
                ::operator delete[](data, std::align_val_t{ alignof(T) }, std::nothrow);

            data = new_storage;
            read_ptr = new_storage + num_items_read();
            write_ptr = new_storage + num_items_written();
            capacity = n;
        }

        return true;
    }
};

template<typename K>
using CacheKeyTrait = std::void_t<
    decltype(K::GetHash(std::declval<K>())),
    decltype(K::CompareKey(std::declval<K>(), std::declval<K>()))
>;

template<typename K, typename V, typename = void>
struct GnCacheTable {};

template<typename K, typename V>
struct GnCacheTable<K, V, CacheKeyTrait<K>>
{
    struct KeyFunctionWrapper
    {
        inline size_t operator()(const K& key) const
        {
            return K::GetHash(key);
        }

        inline bool operator()(const K& a, const K& b) const
        {
            return K::CompareKey(a, b);
        }
    };

    using HashMap = std::unordered_map<K, V, KeyFunctionWrapper, KeyFunctionWrapper>;// TODO: Implement hash map for the cache table

    HashMap cache_table;
    mutable std::shared_mutex mutex;

    inline std::optional<V> Get(const K& key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        auto item = cache_table.find(key);

        if (item != cache_table.end())
            return { item->second };

        return {};
    }

    inline bool Insert(const K& key, const V& value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        cache_table.emplace(key, value);
        return true; // TODO: Return false when fail
    }

    template<typename Fn>
    inline void Flush(Fn&& destroy_fn)
    {
        for (auto& [_, obj] : cache_table) {
            destroy_fn(obj);
        }
    }
};

template<typename T>
struct GnTrackedResource
{
    GnTrackedResource<T>* prev = nullptr;
    GnTrackedResource<T>* next = nullptr;

    void PushTrackedResource(GnTrackedResource<T>* item) noexcept
    {
        item->next = next;
        item->prev = this;
        if (next) next->prev = item;
        next = item;
    }

    GnTrackedResource<T>* PopTrackedResource() noexcept
    {
        auto ret = next;

        if (ret == nullptr) return nullptr;

        next = next->next;
        if (next) next->prev = ret->prev;
        ret->prev = nullptr;
        ret->next = nullptr;

        return ret;
    }

    void RemoveTrackedResource() noexcept
    {
        prev->next = next;
        if (next) next->prev = prev;
        prev = nullptr;
        next = nullptr;
    }
};

template<typename T>
struct GnCachedItem
{

};

struct GnUpdateRange
{
    // 16-bit to save space
    uint16_t first = UINT16_MAX;
    uint16_t last = UINT16_MAX;

    inline void Update(uint32_t idx) noexcept
    {
        if (first == 0xFFFF) {
            first = (uint16_t)idx;
            last = (uint16_t)idx + 1;
            return;
        }

        if (idx < first) first = (uint32_t)idx;
        if (idx > last) last = idx + 1;
    }

    inline void Update(uint32_t first_idx, uint32_t last_idx) noexcept
    {
        if (first == 0xFFFF) {
            first = (uint16_t)first_idx;
            last = (uint16_t)last_idx + 1;
            return;
        }

        if (first_idx < first) first = (uint16_t)first_idx;
        if (last_idx > last) last = (uint16_t)last_idx + 1;
    }

    inline void Flush() noexcept
    {
        first = UINT16_MAX;
        last = UINT16_MAX;
    }
};

template<typename ObjectTypes>
struct GnObjectPool
{
    std::optional<GnPool<typename ObjectTypes::Queue>>                  queue;
    std::optional<GnPool<typename ObjectTypes::Fence>>                  fence;
    std::optional<GnPool<typename ObjectTypes::Memory>>                 memory;
    std::optional<GnPool<typename ObjectTypes::Buffer>>                 buffer;
    std::optional<GnPool<typename ObjectTypes::Texture>>                texture;
    std::optional<GnPool<typename ObjectTypes::TextureView>>            texture_view;
    std::optional<GnPool<typename ObjectTypes::RenderGraph>>            render_graph;
    std::optional<GnPool<typename ObjectTypes::DescriptorTableLayout>>  resource_table_layout;
    std::optional<GnPool<typename ObjectTypes::PipelineLayout>>         pipeline_layout;
    std::optional<GnPool<typename ObjectTypes::DescriptorPool>>         descriptor_pool;
    std::optional<GnPool<typename ObjectTypes::Pipeline>>               pipeline;
    std::optional<GnPool<typename ObjectTypes::CommandPool>>            command_pool;
};

struct GnUnimplementedType {};

#endif // GN_CORE_H_
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

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_ASSERT(x) assert(x)

#define GN_CHECK(x) GN_DBG_ASSERT(x)
#define GN_UNREACHABLE() GN_ASSERT(false && "Unreachable")

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

#define GN_MAX_QUEUE 4

// The maximum number of bound resources of each type on resource table here to prevent resource table abuse
// We may change this number in the future
#define GN_MAX_RESOURCE_TABLE_SAMPLERS 2048u
#define GN_MAX_RESOURCE_TABLE_DESCRIPTORS 1048576u

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

template<typename T, typename T2, typename... Rest>
struct GnUnionStorage
{
    static constexpr std::size_t size = GnMax(sizeof(T), sizeof(T2), sizeof(Rest)...);
    static constexpr std::size_t alignment = GnMax(alignof(T), alignof(T2), alignof(Rest)...);
    alignas(alignment) uint8_t data[size];
};

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

    GnPool(std::size_t objects_per_block) noexcept :
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
        if (num_allocated >= num_reserved) {
            if (!_reserve_new_block()) {
                return nullptr;
            }
        }

        GnPoolChunk* free_chunk = current_allocation;
        last_allocation = current_allocation;
        current_allocation = current_allocation->next_chunk;
        ++num_allocated;

        return free_chunk;
    }

    void free(void* ptr) noexcept
    {
        GnPoolChunk* chunk = reinterpret_cast<GnPoolChunk*>(ptr);
        chunk->next_chunk = current_allocation;
        current_allocation = chunk;
        --num_allocated;
    }

    bool _reserve_new_block() noexcept
    {
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

// Only use this for POD structs!!
template<typename PODType, size_t Size, std::enable_if_t<std::is_pod_v<PODType>, bool> = true>
struct GnSmallVector
{
    PODType local_storage[Size]{};
    PODType* storage = nullptr;
    size_t size = 0;
    size_t capacity = Size;

    GnSmallVector() noexcept :
        storage(local_storage)
    {
    }

    ~GnSmallVector()
    {
        if (storage != local_storage) {
            std::free(storage);
        }
    }

    GnSmallVector& operator=(const GnSmallVector& other)
    {
        if (other.storage != other.local_storage)
            reserve(other.size);

        std::memcpy(storage, other.storage, other.size * sizeof(PODType));
        size = other.size;
        capacity = other.capacity;
        return *this;
    }

    GnSmallVector& operator=(GnSmallVector&& other)
    {
        if (other.storage == other.local_storage)
            std::memcpy(storage, other.storage, other.size * sizeof(PODType));
        else
            storage = other.storage;

        size = other.size;
        capacity = other.capacity;
        std::memset(other.storage, 0, other.size * sizeof(PODType));
        other.size = 0;
        other.capacity = Size;
        return *this;
    }

    PODType& operator[](size_t n) noexcept
    {
        return storage[n];
    }

    const PODType& operator[](size_t n) const noexcept
    {
        return storage[n];
    }

    bool push_back(const PODType& value) noexcept
    {
        if (size == capacity)
            if (!reserve(capacity + 1))
                return false;

        storage[size++] = value;
        return true;
    }

    bool resize(size_t n) noexcept
    {
        size = n;
        return reserve(n);
    }

    bool reserve(size_t n) noexcept
    {
        if (n > capacity) {
            PODType* new_storage = (PODType*)std::malloc(n * sizeof(PODType));
            
            std::memset(new_storage, 0, n * sizeof(PODType));

            if (new_storage == nullptr)
                return false;

            std::memcpy(new_storage, storage, size * sizeof(PODType));

            if (storage != local_storage) {
                std::free(storage);
            }

            storage = new_storage;
            capacity = n;
        }

        return true;
    }
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
    std::optional<GnPool<typename ObjectTypes::ResourceTableLayout>>    resource_table_layout;
    std::optional<GnPool<typename ObjectTypes::PipelineLayout>>         pipeline_layout;
};

struct GnUnimplementedType {};

#endif // GN_CORE_H_
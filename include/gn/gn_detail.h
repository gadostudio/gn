#ifndef GN_DETAIL_H_
#define GN_DETAIL_H_

#include <type_traits>
#include <variant>
#include <optional>
#include <array>
#include <cstdint>
#include <cassert>

#define GN_MAX_CHARS 256

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_CHECK(x) GN_DBG_ASSERT(x)

#define GN_ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

namespace gn::detail
{
    using LimitVariant = std::variant<uint32_t, float>;

    template<size_t Count>
    using LimitMap = std::array<LimitVariant, Count>;

    template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
    static constexpr size_t enum_count = static_cast<size_t>(EnumType::Count);

    template<typename T, size_t Size>
    class StaticQueue
    {
    public:
        StaticQueue()
        {
        }

    private:

    };

    template<typename T>
    inline static T min(T a, T b)
    {
        return b > a ? b : a;
    }

    template<typename T>
    inline static T max(T a, T b)
    {
        return a > b ? a : b;
    }

    template<typename EnumType, std::enable_if_t<std::is_enum_v<EnumType>, bool> = true>
    inline static size_t as_size_t(EnumType e)
    {
        return static_cast<size_t>(e);
    }
}

#endif // GN_DETAIL_H_
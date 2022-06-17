#ifndef GN_DETAIL_H_
#define GN_DETAIL_H_

#ifdef NDEBUG
#define GN_DBG_ASSERT(x)
#else
#define GN_DBG_ASSERT(x) assert(x)
#endif

#define GN_CHECK(x) GN_DBG_ASSERT(x)

namespace gn::detail
{
    template<typename T, size_t Size>
    class StaticQueue
    {
    public:
        StaticQueue()
        {
        }

    private:

    };
}

#endif // GN_DETAIL_H_
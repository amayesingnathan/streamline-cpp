#pragma once

#include "slc/Coroutine/Enumerable.h"

#include <map>

namespace slc {

    template<typename TKey, typename TValue,
        typename Compare = std::less<TKey>,
        typename Allocator = std::allocator<std::pair<const TKey, TValue>>>
    class Map : public std::map<TKey, TValue, Compare, Allocator>, public IEnumerable<std::pair<const TKey, TValue>>
    {
    public:
        using std::map<TKey, TValue, Compare, Allocator>::map;
        using std::map<TKey, TValue, Compare, Allocator>::begin;
        using std::map<TKey, TValue, Compare, Allocator>::end;

        MAKE_RANGE_ENUMERABLE(Map)
    };

}
#pragma once

#include "Coroutine/Enumerable.h"

#include <unordered_map>

namespace slc {

    template<typename TKey, typename TValue,
        typename Hash = std::hash<TKey>,
        typename KeyEqual = std::equal_to<TKey>,
        typename Allocator = std::allocator<TKey>>
        class HashMap : public std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>, public IEnumerable<std::pair<const TKey, TValue>>
    {
    public:
        using std::unordered_map<TKey, TValue, Hash, KeyEqual, Allocator>::unordered_map;

        MAKE_RANGE_ENUMERABLE(HashMap)
    };

}
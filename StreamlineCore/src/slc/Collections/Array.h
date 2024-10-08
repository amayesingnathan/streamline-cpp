#pragma once

#include "slc/Coroutine/Enumerable.h"

#include <array>

namespace slc {

    template<typename T, size_t TSize>
    class Array : public std::array<T, TSize>, public IEnumerable<T>
    {
    public:
        using std::array<T, TSize>::array;
        using std::array<T, TSize>::begin;
        using std::array<T, TSize>::end;

        Array() { memset(this->data(), 0, TSize * sizeof(T)); }
        Array(std::array<T, TSize>&& arr) :
            std::array<T, TSize>(std::move(arr)) {}

        MAKE_RANGE_ENUMERABLE(Array)

    };
}
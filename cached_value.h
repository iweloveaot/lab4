#ifndef CACHED_VALUE_H
#define CACHED_VALUE_H

#include "ordinal_index.h"

template<typename T>
struct CachedValue {
    OrdinalIndex index;
    T value;

    CachedValue() {}
    CachedValue(const OrdinalIndex& idx, const T& val) : index(idx), value(val) {}
};

#endif
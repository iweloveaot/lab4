#ifndef GENERATOR_RANGE_H
#define GENERATOR_RANGE_H

#include "ordinal_index.h"

template<typename T> class IGenerator;

template<typename T>
struct GeneratorRange {
    OrdinalIndex start;
    OrdinalIndex end;
    IGenerator<T>* generator;
    int localOffset; // НОВОЕ: С какого локального индекса генератора начинается этот диапазон

    GeneratorRange() : generator(nullptr), localOffset(0) {}
    GeneratorRange(const OrdinalIndex& s, const OrdinalIndex& e, IGenerator<T>* gen, int offset = 0)
        : start(s), end(e), generator(gen), localOffset(offset) {}
};

#endif
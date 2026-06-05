#ifndef GENERATOR_RANGE_H
#define GENERATOR_RANGE_H

#include "ordinal_index.h"

template<typename T> class IGenerator;

template<typename T>
struct GeneratorRange {
    OrdinalIndex start;
    OrdinalIndex end;
    IGenerator<T>* generator;

    GeneratorRange() : generator(nullptr) {}
    GeneratorRange(const OrdinalIndex& s, const OrdinalIndex& e, IGenerator<T>* gen)
        : start(s), end(e), generator(gen) {}
};

#endif
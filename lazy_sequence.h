#ifndef LAZY_SEQUENCE_H
#define LAZY_SEQUENCE_H

#include "composite_generator.h"
#include "cached_value.h"
#include "base/mutable_array_sequence.h"
#include "igenerator.h"

template<typename T>
class LazySequence {
private:
    CompositeGenerator<T>* generator;
    MutableArraySequence<CachedValue<T>>* cache;

public:
    LazySequence(IGenerator<T>* rootGenerator);
    ~LazySequence();

    // Запрещаем копирование
    LazySequence(const LazySequence&) = delete;
    LazySequence& operator=(const LazySequence&) = delete;

    T Get(const OrdinalIndex& index) const;
    void Prepend(const T& value);
    void Append(const T& value);
    void InsertAt(const OrdinalIndex& index, const T& value);
    void Remove(const OrdinalIndex& index);
    void InsertSequence(const OrdinalIndex& index, IGenerator<T>* seq);
    void Concat(const LazySequence<T>& other);

    bool IsCached(const OrdinalIndex& index) const;
    int CachedCount() const;
    int RangeCount() const;

private:
    int FindCached(const OrdinalIndex& index) const;
    void ShiftCachedAfter(const OrdinalIndex& border, int delta);
};

#include "lazy_sequence.tpp"

#endif
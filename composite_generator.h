#ifndef COMPOSITE_GENERATOR_H
#define COMPOSITE_GENERATOR_H

#include "generator_range.h"
#include "ordinal_index.h"
#include "base/mutable_array_sequence.h"
#include "igenerator.h"

template<typename T>
class CompositeGenerator {
private:
    MutableArraySequence<GeneratorRange<T>>* ranges;
    MutableArraySequence<MutableArraySequence<int>*>* removedIndices;

public:
    CompositeGenerator();
    ~CompositeGenerator();

    // Запрещаем копирование, чтобы избежать double-free
    CompositeGenerator(const CompositeGenerator&) = delete;
    CompositeGenerator& operator=(const CompositeGenerator&) = delete;

    void AddRange(const GeneratorRange<T>& range);
    int GetRangeCount() const;
    const GeneratorRange<T>& GetRange(int index) const;
    GeneratorRange<T>& GetRangeMutable(int index);

    int FindRange(const OrdinalIndex& index) const;
    
    T Generate(const OrdinalIndex& index) const;
    void Remove(const OrdinalIndex& index);

    void ShiftAllRanges(int count);
    void ShiftRangesAfter(const OrdinalIndex& border, int delta);
    
    void InsertSequence(const OrdinalIndex& start, IGenerator<T>* generator);
    void Concat(const CompositeGenerator<T>& other);

private:
    int CountRemovedBefore(int rangeIndex, int trueLocalIndex) const;
    int ToGeneratorIndex(int rangeIndex, int trueLocalIndex) const;
};

#include "composite_generator.tpp"

#endif
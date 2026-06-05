#ifndef COMPOSITE_GENERATOR_TPP
#define COMPOSITE_GENERATOR_TPP

#include "composite_generator.h"
#include <stdexcept>

template<typename T>
CompositeGenerator<T>::CompositeGenerator() {
    ranges = new MutableArraySequence<GeneratorRange<T>>();
    removedIndices = new MutableArraySequence<MutableArraySequence<int>*>();
}

template<typename T>
CompositeGenerator<T>::~CompositeGenerator() {
    for(int i = 0; i < ranges->GetLength(); i++) delete ranges->Get(i).generator;
    for(int i = 0; i < removedIndices->GetLength(); i++) delete removedIndices->Get(i);
    delete ranges;
    delete removedIndices;
}

template<typename T>
void CompositeGenerator<T>::AddRange(const GeneratorRange<T>& range) {
    ranges->Append(range);
    removedIndices->Append(new MutableArraySequence<int>());
}

template<typename T>
int CompositeGenerator<T>::GetRangeCount() const { return ranges->GetLength(); }

template<typename T>
const GeneratorRange<T>& CompositeGenerator<T>::GetRange(int index) const { return ranges->Get(index); }

template<typename T>
GeneratorRange<T>& CompositeGenerator<T>::GetRangeMutable(int index) { 
    // Снимаем константность, так как нам нужно менять границы диапазонов in-place
    return const_cast<GeneratorRange<T>&>(ranges->Get(index)); 
}

// Ищем с конца, чтобы новые (вставленные) диапазоны перекрывали старые
template<typename T>
int CompositeGenerator<T>::FindRange(const OrdinalIndex& index) const {
    for(int i = ranges->GetLength() - 1; i >= 0; i--) {
        const auto& r = ranges->Get(i);
        if(index >= r.start && index < r.end) return i;
    }
    return -1;
}

template<typename T>
int CompositeGenerator<T>::CountRemovedBefore(int rangeIndex, int trueLocalIndex) const {
    int count = 0;
    auto list = removedIndices->Get(rangeIndex);
    for(int i = 0; i < list->GetLength(); i++) {
        if(list->Get(i) <= trueLocalIndex) count++;
    }
    return count;
}

template<typename T>
int CompositeGenerator<T>::ToGeneratorIndex(int rangeIndex, int trueLocalIndex) const {
    return trueLocalIndex + CountRemovedBefore(rangeIndex, trueLocalIndex);
}

template<typename T>
T CompositeGenerator<T>::Generate(const OrdinalIndex& index) const {
    int rangeIndex = FindRange(index);
    if(rangeIndex < 0) throw std::out_of_range("Generator range not found");

    const auto& range = ranges->Get(rangeIndex);
    
    // ИСПРАВЛЕНИЕ: Вычитаем старт диапазона для получения истинного локального индекса
    int trueLocalIndex = index.finitePart - range.start.finitePart;
    int genIndex = ToGeneratorIndex(rangeIndex, trueLocalIndex);
    
    return range.generator->Generate(genIndex);
}

template<typename T>
void CompositeGenerator<T>::Remove(const OrdinalIndex& index) {
    int rangeIndex = FindRange(index);
    if(rangeIndex < 0) return;

    const auto& range = ranges->Get(rangeIndex);
    // ИСПРАВЛЕНИЕ: Сохраняем именно истинный локальный индекс
    int trueLocalIndex = index.finitePart - range.start.finitePart;
    
    removedIndices->Get(rangeIndex)->Append(trueLocalIndex);
}

template<typename T>
void CompositeGenerator<T>::ShiftAllRanges(int count) {
    for(int i = 0; i < ranges->GetLength(); i++) {
        auto& range = GetRangeMutable(i);
        range.start.finitePart += count;
        range.end.finitePart += count;
    }
}

template<typename T>
void CompositeGenerator<T>::ShiftRangesAfter(const OrdinalIndex& border, int delta) {
    for(int i = 0; i < ranges->GetLength(); i++) {
        auto& range = GetRangeMutable(i);
        if(range.start > border) range.start.finitePart += delta;
        if(range.end > border) range.end.finitePart += delta;
    }
}

template<typename T>
void CompositeGenerator<T>::InsertSequence(const OrdinalIndex& start, IGenerator<T>* generator) {
    OrdinalIndex end(start.omegaPart + 1, 0);
    AddRange(GeneratorRange<T>(start, end, generator->Clone()));
}

template<typename T>
void CompositeGenerator<T>::Concat(const CompositeGenerator<T>& other) {
    for(int i = 0; i < other.GetRangeCount(); i++) {
        const auto& r = other.GetRange(i);
        ranges->Append(GeneratorRange<T>(r.start, r.end, r.generator->Clone()));
        
        // Глубокое копирование удаленных индексов
        auto* otherRemoved = other.removedIndices->Get(i);
        auto* newRemoved = new MutableArraySequence<int>();
        for(int j = 0; j < otherRemoved->GetLength(); j++) {
            newRemoved->Append(otherRemoved->Get(j));
        }
        removedIndices->Append(newRemoved);
    }
}

#endif
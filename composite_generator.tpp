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
CompositeGenerator<T>::CompositeGenerator(const CompositeGenerator& other) {
    ranges = new MutableArraySequence<GeneratorRange<T>>();
    removedIndices = new MutableArraySequence<MutableArraySequence<int>*>();
    
    for(int i = 0; i < other.ranges->GetLength(); i++) {
        const auto& r = other.ranges->Get(i);
        ranges->Append(GeneratorRange<T>(r.start, r.end, r.generator->Clone(), r.localOffset));
        
        auto* otherRemoved = other.removedIndices->Get(i);
        auto* newRemoved = new MutableArraySequence<int>();
        for(int j = 0; j < otherRemoved->GetLength(); j++) {
            newRemoved->Append(otherRemoved->Get(j));
        }
        removedIndices->Append(newRemoved);
    }
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
    return const_cast<GeneratorRange<T>&>(ranges->Get(index)); 
}

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
    
    int trueLocalIndex = range.localOffset + (index.finitePart - range.start.finitePart);
    int genIndex = ToGeneratorIndex(rangeIndex, trueLocalIndex);
    
    return range.generator->Generate(genIndex);
}

template<typename T>
void CompositeGenerator<T>::Remove(const OrdinalIndex& index) {
    int rangeIndex = FindRange(index);
    if(rangeIndex < 0) return;

    const auto& range = ranges->Get(rangeIndex);
    int trueLocalIndex = range.localOffset + (index.finitePart - range.start.finitePart);
    removedIndices->Get(rangeIndex)->Append(trueLocalIndex);
}

template<typename T>
void CompositeGenerator<T>::ShiftAllRanges(int count) {
    for(int i = 0; i < ranges->GetLength(); i++) {
        auto& range = GetRangeMutable(i);
        
        if (range.start.omegaPart == 0) {
            range.start.finitePart += count;
        }
        if (range.end.omegaPart == 0 && range.end.finitePart != 0) {
            range.end.finitePart += count;
        }
    }
}

template<typename T>
void CompositeGenerator<T>::ShiftRangesAfter(const OrdinalIndex& border, int delta) {
    for(int i = 0; i < ranges->GetLength(); i++) {
        auto& range = GetRangeMutable(i);

        if(range.start.omegaPart == border.omegaPart && range.start.finitePart > border.finitePart) {
            range.start.finitePart += delta;
        }
        
        if(range.end.omegaPart == border.omegaPart && range.end.finitePart > border.finitePart) {
            range.end.finitePart += delta;
        }
    }
}

template<typename T>
OrdinalIndex CompositeGenerator<T>::GetMaxEnd() const {
    OrdinalIndex maxEnd(0, 0);
    for(int i = 0; i < ranges->GetLength(); i++) {
        if(ranges->Get(i).end > maxEnd) {
            maxEnd = ranges->Get(i).end;
        }
    }
    return maxEnd;
}

template<typename T>
void CompositeGenerator<T>::InsertSequence(const OrdinalIndex& X, IGenerator<T>* newSeqGen) {
    int splitRangeIdx = -1;
    for(int i = 0; i < ranges->GetLength(); i++) {
        const auto& r = ranges->Get(i);
        if (X > r.start && X < r.end) {
            splitRangeIdx = i;
            break;
        }
    }

    for(int i = 0; i < ranges->GetLength(); i++) {
        auto& r = GetRangeMutable(i);
        if (r.start >= X) {
            r.start = r.start.ShiftByOmega(X);
            r.end = r.end.ShiftByOmega(X);
        }
    }

    if (splitRangeIdx != -1) {
        auto& origRange = GetRangeMutable(splitRangeIdx);
        
        OrdinalIndex origEnd = origRange.end;
        int origOffset = origRange.localOffset;
        IGenerator<T>* origGen = origRange.generator;

        origRange.end = X;
        OrdinalIndex newStart = X.ShiftByOmega(X); 
        OrdinalIndex newEnd = origEnd.ShiftByOmega(X);
        int newOffset = origOffset + (X.finitePart - origRange.start.finitePart);
        
        AddRange(GeneratorRange<T>(newStart, newEnd, origGen->Clone(), newOffset));
    }

    OrdinalIndex newSeqEnd = X.ShiftByOmega(X);
    AddRange(GeneratorRange<T>(X, newSeqEnd, newSeqGen->Clone(), 0));
}

template<typename T>
void CompositeGenerator<T>::Concat(const CompositeGenerator<T>& other) {
    for(int i = 0; i < other.GetRangeCount(); i++) {
        const auto& r = other.GetRange(i);
        ranges->Append(GeneratorRange<T>(r.start, r.end, r.generator->Clone(), r.localOffset));
        
        auto* otherRemoved = other.removedIndices->Get(i);
        auto* newRemoved = new MutableArraySequence<int>();
        for(int j = 0; j < otherRemoved->GetLength(); j++) {
            newRemoved->Append(otherRemoved->Get(j));
        }
        removedIndices->Append(newRemoved);
    }
}

template<typename T>
void CompositeGenerator<T>::ConcatWithShift(const CompositeGenerator<T>& other, const OrdinalIndex& shift) {
    for(int i = 0; i < other.ranges->GetLength(); i++) {
        const auto& r = other.ranges->Get(i);
        
        OrdinalIndex newStart = r.start.ShiftBy(shift);
        OrdinalIndex newEnd = r.end.ShiftBy(shift);
        ranges->Append(GeneratorRange<T>(newStart, newEnd, r.generator->Clone(), r.localOffset));
        auto* otherRemoved = other.removedIndices->Get(i);
        auto* newRemoved = new MutableArraySequence<int>();
        for(int j = 0; j < otherRemoved->GetLength(); j++) {
            newRemoved->Append(otherRemoved->Get(j));
        }
        removedIndices->Append(newRemoved);
    }
}

template<typename T>
CompositeGenerator<T>& CompositeGenerator<T>::operator=(const CompositeGenerator& other) {
    if (this != &other) {
        for(int i = 0; i < ranges->GetLength(); i++) delete ranges->Get(i).generator;
        for(int i = 0; i < removedIndices->GetLength(); i++) delete removedIndices->Get(i);
        delete ranges;
        delete removedIndices;
        
        ranges = new MutableArraySequence<GeneratorRange<T>>();
        removedIndices = new MutableArraySequence<MutableArraySequence<int>*>();
        
        for(int i = 0; i < other.ranges->GetLength(); i++) {
            const auto& r = other.ranges->Get(i);
            ranges->Append(GeneratorRange<T>(r.start, r.end, r.generator->Clone(), r.localOffset));
            
            auto* otherRemoved = other.removedIndices->Get(i);
            auto* newRemoved = new MutableArraySequence<int>();
            for(int j = 0; j < otherRemoved->GetLength(); j++) {
                newRemoved->Append(otherRemoved->Get(j));
            }
            removedIndices->Append(newRemoved);
        }
    }
    return *this;
}

#endif
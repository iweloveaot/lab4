#ifndef LAZY_SEQUENCE_TPP
#define LAZY_SEQUENCE_TPP

#include "lazy_sequence.h"

template<typename T>
LazySequence<T>::LazySequence(IGenerator<T>* rootGenerator) {
    generator = new CompositeGenerator<T>();
    cache = new MutableArraySequence<CachedValue<T>>();
    
    generator->AddRange(GeneratorRange<T>(
        OrdinalIndex(0,0), OrdinalIndex(1,0), rootGenerator->Clone()
    ));
}

template<typename T>
LazySequence<T>::~LazySequence() {
    delete generator;
    delete cache;
}

template<typename T>
int LazySequence<T>::FindCached(const OrdinalIndex& index) const {
    for(int i = 0; i < cache->GetLength(); i++) {
        if(cache->Get(i).index == index) return i;
    }
    return -1;
}

template<typename T>
void LazySequence<T>::ShiftCachedAfter(const OrdinalIndex& border, int delta) {
    for(int i = 0; i < cache->GetLength(); i++) {
        if(cache->Get(i).index > border) {
            // Снимаем константность для in-place сдвига
            const_cast<CachedValue<T>&>(cache->Get(i)).index.finitePart += delta;
        }
    }
}

template<typename T>
T LazySequence<T>::Get(const OrdinalIndex& index) const {
    int pos = FindCached(index);
    if(pos >= 0) return cache->Get(pos).value;
    return generator->Generate(index);
}

template<typename T>
void LazySequence<T>::Prepend(const T& value) {
    for(int i = 0; i < cache->GetLength(); i++) {
        const_cast<CachedValue<T>&>(cache->Get(i)).index.finitePart++;
    }
    generator->ShiftAllRanges(1);
    cache->Append(CachedValue<T>(OrdinalIndex(0,0), value));
}

template<typename T>
void LazySequence<T>::Append(const T& value) {
    // Ищем максимальный индекс строго в омега-зоне (omegaPart >= 1)
    // Если таких нет, maxIdx останется (1, -1), и newIdx станет (1, 0) = ω
    OrdinalIndex maxIdx(1, -1); 
    
    for(int i = 0; i < cache->GetLength(); i++) {
        OrdinalIndex idx = cache->Get(i).index;
        // Учитываем только те элементы, которые уже находятся в зоне ω или выше
        if(idx.omegaPart >= 1 && idx > maxIdx) {
            maxIdx = idx;
        }
    }
    
    OrdinalIndex newIdx = maxIdx + 1;
    cache->Append(CachedValue<T>(newIdx, value));
}

template<typename T>
void LazySequence<T>::InsertAt(const OrdinalIndex& index, const T& value) {
    ShiftCachedAfter(index, 1);
    generator->ShiftRangesAfter(index, 1);
    cache->Append(CachedValue<T>(index, value));
}

template<typename T>
void LazySequence<T>::Remove(const OrdinalIndex& index) {
    int cachedPos = FindCached(index);
    if(cachedPos >= 0) {
        cache->RemoveAt(cachedPos);
        ShiftCachedAfter(index, -1);
        generator->ShiftRangesAfter(index, -1);
        return;
    }
    generator->Remove(index);
}

template<typename T>
void LazySequence<T>::InsertSequence(const OrdinalIndex& index, IGenerator<T>* seq) {
    for(int i = 0; i < cache->GetLength(); i++) {
        if(cache->Get(i).index >= index) {
            const_cast<CachedValue<T>&>(cache->Get(i)).index.omegaPart++;
        }
    }
    for(int i = 0; i < generator->GetRangeCount(); i++) {
        auto& range = generator->GetRangeMutable(i);
        if(range.start >= index) range.start.omegaPart++;
        if(range.end >= index) range.end.omegaPart++;
    }
    generator->InsertSequence(index, seq);
}

template<typename T>
void LazySequence<T>::Concat(const LazySequence<T>& other) {
    generator->Concat(*other.generator);
    for(int i = 0; i < other.cache->GetLength(); i++) {
        cache->Append(other.cache->Get(i));
    }
}

template<typename T>
bool LazySequence<T>::IsCached(const OrdinalIndex& index) const { return FindCached(index) >= 0; }

template<typename T>
int LazySequence<T>::CachedCount() const { return cache->GetLength(); }

template<typename T>
int LazySequence<T>::RangeCount() const { return generator->GetRangeCount(); }

#endif
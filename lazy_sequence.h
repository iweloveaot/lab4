#ifndef LAZY_SEQUENCE_H
#define LAZY_SEQUENCE_H

#include "composite_generator.h"
#include "cached_value.h"
#include "base/mutable_array_sequence.h"
#include "igenerator.h"
#include "base/exceptions.h"

template<typename T>
class LazySequence: public Sequence<T> {
private:
    CompositeGenerator<T>* generator;
    MutableArraySequence<CachedValue<T>>* cache;

    int FindCached(const OrdinalIndex& index) const {
        for(int i = 0; i < cache->GetLength(); i++) {
            if(cache->Get(i).index == index) return i;
        }
        return -1;
    }

    void ShiftCachedAfter(const OrdinalIndex& border, int delta) {
        for(int i = 0; i < cache->GetLength(); i++) {
            auto& idx = const_cast<CachedValue<T>&>(cache->Get(i)).index;
            if (idx.omegaPart == border.omegaPart && idx.finitePart > border.finitePart) {
                idx.finitePart += delta;
            }
        }
    }

    class LazyEnumerator : public IEnumerator<T> {
    private:
        const LazySequence<T>* seq;
        int currentIndex;
    public:
        LazyEnumerator(const LazySequence<T>* s) 
            : seq(s), currentIndex(-1) {}
        
        bool HasNext() override {
            ++currentIndex;
            return true; 
        }

        const T& GetCurrent() const override {
            if (currentIndex < 0) throw IndexOutOfRangeException("Enumerator not positioned");
            return seq->Get(currentIndex);
        }

        void Reset() override {
            currentIndex = -1;
        }
    };

    void PrependMutate(const T& value) {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }
        for(int i = 0; i < cache->GetLength(); i++) {
            auto& idx = const_cast<CachedValue<T>&>(cache->Get(i)).index;
            if (idx.omegaPart == 0) {
                idx.finitePart++;
            }
        }
        generator->ShiftAllRanges(1);
        cache->Append(CachedValue<T>(OrdinalIndex(0,0), value));
    }

    void AppendMutate(const T& value) {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }
        OrdinalIndex maxCached(1, -1); 
        for(int i = 0; i < cache->GetLength(); i++) {
            OrdinalIndex idx = cache->Get(i).index;
            if(idx.omegaPart >= 1 && idx > maxCached) {
                maxCached = idx;
            }
        }
        
        OrdinalIndex genMaxEnd = generator->GetMaxEnd();
        OrdinalIndex newIdx;
        
        if (genMaxEnd > maxCached) {
            newIdx = genMaxEnd; 
        } else {
            newIdx = maxCached + 1;
        }
        
        cache->Append(CachedValue<T>(newIdx, value));
    }

    void InsertAtMutate(const OrdinalIndex& index, const T& value) {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }
        ShiftCachedAfter(index, 1);
        generator->ShiftRangesAfter(index, 1);
        cache->Append(CachedValue<T>(index, value));
    }

    void RemoveMutate(const OrdinalIndex& index) {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }
        int cachedPos = FindCached(index);
        if(cachedPos >= 0) {
            cache->RemoveAt(cachedPos);
            ShiftCachedAfter(index, -1);
            generator->ShiftRangesAfter(index, -1);
            return;
        }
        generator->Remove(index);
    }

    void InsertSequenceMutate(const OrdinalIndex& index, IGenerator<T>* seq) {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }
        for(int i = 0; i < cache->GetLength(); i++) {
            if(cache->Get(i).index >= index) {
                auto& cachedIdx = const_cast<CachedValue<T>&>(cache->Get(i)).index;
                cachedIdx = cachedIdx.ShiftByOmega(index);
            }
        }
        generator->InsertSequence(index, seq);
    }

protected:
    virtual Sequence<T>* Instance() override {
        return new LazySequence<T>(*this);
    }

    virtual Sequence<T>* PrependImplict(const T &item) override {
        return PrependLazy(item);
    }

    virtual Sequence<T>* AppendImplict(const T &item) override {
        return AppendLazy(item);
    }

    virtual Sequence<T>* InsertAtImplict(const T &item, int index) override {
        return InsertAtLazy(OrdinalIndex(0, index), item);
    }
    
    virtual Sequence<T>* RemoveAtImplict(int index) override {
        return RemoveLazy(OrdinalIndex(0, index));
    }

    virtual Sequence<T>* ConcatImplict(const Sequence<T> *other) override {
        if (!other) throw NullReferenceException("Null sequence in Concat");
        int len = other->GetLength();
        if (len < 0) throw BaseException("Cannot concat infinite sequence to LazySequence");
        LazySequence<T>* result = new LazySequence<T>(*this);
        for (int i = 0; i < len; i++) {
            result->AppendMutate(other->Get(i));
        }
        return result;
    }

public:
    LazySequence() 
        : generator(nullptr), cache(new MutableArraySequence<CachedValue<T>>()) {
    }

    LazySequence(IGenerator<T>* rootGenerator) {
        generator = new CompositeGenerator<T>();
        cache = new MutableArraySequence<CachedValue<T>>();
        
        generator->AddRange(GeneratorRange<T>(
            OrdinalIndex(0,0), OrdinalIndex(1,0), rootGenerator->Clone(), 0
        ));
    }

    LazySequence(const LazySequence<T>& other) {
        generator = new CompositeGenerator<T>(*other.generator);
        cache = new MutableArraySequence<CachedValue<T>>(*other.cache);
    }

    ~LazySequence() {
        delete generator;
        delete cache;
    }

    const T& GetFirst() const {
        return Get(0);
    }

    const T& GetLast() const {
        throw BaseException("LazySequence is infinite and has no last element");
    }

    const T& Get(int index) const {
        if (index < 0) throw IndexOutOfRangeException("Negative index in Get");
        return Get(OrdinalIndex(0, index));
    }

    Sequence<T>* GetSubsequence(int startIndex, int endIndex) const {
        if (startIndex < 0 || endIndex < startIndex) {
            throw IndexOutOfRangeException("Invalid subsequence indexes");
        }
        auto* sub = new MutableArraySequence<T>();
        for (int i = startIndex; i <= endIndex; i++) {
            sub->Append(this->Get(i));
        }
        return sub;
    }

    int GetLength() const {
        return -1; // послед-ть бесконечна
    }

    Sequence<T>* Map(T (*func)(const T&)) const {
        throw BaseException("Map is not supported for infinite LazySequence");
    }

    Sequence<T>* Where(bool (*pred)(const T&)) const {
        throw BaseException("Where is not supported for infinite LazySequence");
    }

    void Reduce(T (*func)(const T&, const T&), const T &init, T *result) const {
        throw BaseException("Reduce is not supported for infinite LazySequence");
    }

    Option<T> TryGetFirst(bool (*pred)(const T&)) const {  // охватывает только первые 10.000 элементов
        for (int i = 0; i < 10000; i++) {
            const T& val = Get(i);
            if (!pred || pred(val)) {
                return Option<T>(val);
            }
        }
        return Option<T>::None();
    }

    Option<T> TryGetLast(bool (*pred)(const T&)) const {
        return Option<T>::None(); // у бесконечной последовательности нет последнего элемента
    }

    IEnumerator<T>* GetEnumerator() const {
        return new LazyEnumerator(this);
    }

    const T& Get(const OrdinalIndex& index) const {
        if (!generator) {
            throw BaseException("LazySequence is not initialized with a generator");
        }

        int pos = FindCached(index);
        if (pos >= 0) {
            return cache->Get(pos).value;
        }
        
        T val = generator->Generate(index);
        const_cast<MutableArraySequence<CachedValue<T>>*>(cache)->Append(CachedValue<T>(index, val));
        return cache->Get(cache->GetLength() - 1).value;
    }

    LazySequence<T>* PrependLazy(const T& value) const {
        LazySequence<T>* result = new LazySequence<T>(*this);
        result->PrependMutate(value);
        return result;
    }

    LazySequence<T>* AppendLazy(const T& value) const {
        LazySequence<T>* result = new LazySequence<T>(*this);
        result->AppendMutate(value);
        return result;
    }

    LazySequence<T>* InsertAtLazy(const OrdinalIndex& index, const T& value) const {
        LazySequence<T>* result = new LazySequence<T>(*this);
        result->InsertAtMutate(index, value);
        return result;
    }

    LazySequence<T>* RemoveLazy(const OrdinalIndex& index) const {
        LazySequence<T>* result = new LazySequence<T>(*this);
        result->RemoveMutate(index);
        return result;
    }

    LazySequence<T>* InsertSequence(const OrdinalIndex& index, IGenerator<T>* seq) const {
        LazySequence<T>* result = new LazySequence<T>(*this);
        result->InsertSequenceMutate(index, seq);
        return result;
    }

    LazySequence<T>* ConcatLazy(const LazySequence<T>& other) const {
        LazySequence<T>* result = new LazySequence<T>(*this);

        OrdinalIndex maxEnd = result->generator->GetMaxEnd();
        for(int i = 0; i < result->cache->GetLength(); i++) {
            OrdinalIndex idx = result->cache->Get(i).index;
            OrdinalIndex nextIdx = idx + 1;
            if (nextIdx > maxEnd) {
                maxEnd = nextIdx;
            }
        }
        
        result->generator->ConcatWithShift(*other.generator, maxEnd);
        for(int i = 0; i < other.cache->GetLength(); i++) {
            CachedValue<T> cv = other.cache->Get(i);
            cv.index = cv.index.ShiftBy(maxEnd);
            result->cache->Append(cv);
        }
        
        return result;
    }

    bool IsCached(const OrdinalIndex& index) const { // проверка -- сохранён ли элемент по индексу
        return FindCached(index) >= 0; 
    }

    int CachedCount() const { 
        return cache->GetLength(); 
    }

    int RangeCount() const { 
        return generator->GetRangeCount(); 
    }

    LazySequence<T>& operator=(const LazySequence<T>& other) {
        if (this != &other) {
            delete generator;
            delete cache;
            generator = new CompositeGenerator<T>(*other.generator);
            cache = new MutableArraySequence<CachedValue<T>>(*other.cache);
        }
        return *this;
    }

};

#endif
#ifndef LAZY_SEQUENCE_H
#define LAZY_SEQUENCE_H

#include "base/sequence.h"
#include "base/mutable_array_sequence.h"
#include "base/exceptions.h"
#include "generator.h"


template<typename T>
class LazySequence : public Sequence<T> {
private:
    IGenerator<T>* generator;
    MutableArraySequence<T>* computed_values;
    bool is_finite;
    int finite_length;

protected:
    Sequence<T>* Instance() override {return this;} 

    virtual Sequence<T>* AppendImplict(const T &item) override {
        int new_length = (is_finite ? finite_length : computed_values->GetLength()) + 1;
        IGenerator<T>* new_gen = new AppendGenerator<T>(generator, item, computed_values->GetLength());
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen, new_length);
        
        for (int i = 0; i < computed_values->GetLength(); ++i)
            new_seq->computed_values->Append(computed_values->Get(i));
        new_seq->computed_values->Append(item);
        
        return new_seq;
    }

    virtual Sequence<T>* PrependImplict(const T &item) override {
        int new_length = (is_finite ? finite_length : computed_values->GetLength()) + 1;
        IGenerator<T>* new_gen = new PrependGenerator<T>(generator, item);
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen, new_length);
        
        new_seq->computed_values->Append(item);
        for (int i = 0; i < computed_values->GetLength(); ++i)
            new_seq->computed_values->Append(computed_values->Get(i));
        
        return new_seq;
    }

    virtual Sequence<T>* InsertAtImplict(const T &item, int index) override {
        if (index < 0)
            throw IndexOutOfRangeException("Index out of range");
        
        int current_length = is_finite ? finite_length : computed_values->GetLength();
        if (index > current_length)
            throw IndexOutOfRangeException("Index out of range");
        
        int new_length = current_length + 1;
        IGenerator<T>* new_gen = new InsertAtGenerator<T>(generator, item, index);
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen, new_length);
        
        for (int i = 0; i < computed_values->GetLength(); ++i) {
            if (i == index && i < computed_values->GetLength())
                new_seq->computed_values->Append(item);
            new_seq->computed_values->Append(computed_values->Get(i));
        }
        if (index == computed_values->GetLength())
            new_seq->computed_values->Append(item);
        
        return new_seq;
    }

    virtual Sequence<T>* RemoveAtImplict(int index) override {
        if (index < 0)
            throw IndexOutOfRangeException("Index out of range");
        
        int current_length = is_finite ? finite_length : computed_values->GetLength();
        if (index >= current_length)
            throw IndexOutOfRangeException("Index out of range");
        
        int new_length = current_length - 1;
        IGenerator<T>* new_gen = new RemoveAtGenerator<T>(generator, index);
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen, new_length);
        
        for (int i = 0; i < computed_values->GetLength(); ++i) {
            if (i != index)
                new_seq->computed_values->Append(computed_values->Get(i));
        }
        
        return new_seq;
    }

    virtual Sequence<T>* ConcatImplict(const Sequence<T>* other) override {
        int first_len = is_finite ? finite_length : computed_values->GetLength();
        int second_len = other->GetLength();
        int new_length = first_len + second_len;
        
        IGenerator<T>* new_gen = new ConcatGenerator<T>(generator, other, first_len);
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen, new_length);
        
        for (int i = 0; i < computed_values->GetLength(); ++i)
            new_seq->computed_values->Append(computed_values->Get(i));
        
        int copy_count = (second_len < 10) ? second_len : 10;
        for (int i = 0; i < copy_count && i < other->GetLength(); ++i)
            new_seq->computed_values->Append(other->Get(i));
        
        return new_seq;
    }



public:
    LazySequence(IGenerator<T>* gen)
        : generator(gen), computed_values(new MutableArraySequence<T>()), 
          is_finite(false), finite_length(-1) {}

    LazySequence(T (*rule)(const T&)) 
        : generator(new GeneratorRule<T>(rule)), computed_values(new MutableArraySequence<T>()), 
          is_finite(false), finite_length(-1) {}

    LazySequence(IGenerator<T>* gen, int length)
        : generator(gen), computed_values(new MutableArraySequence<T>()), 
          is_finite(true), finite_length(length) {}

    LazySequence(const LazySequence& other)
        : generator(other.generator->Clone()), 
          computed_values(new MutableArraySequence<T>()),
          is_finite(other.is_finite), finite_length(other.finite_length)
    {
        for (int i = 0; i < other.computed_values->GetLength(); ++i)
            computed_values->Append(other.computed_values->Get(i));
    }

    ~LazySequence() {
        delete generator;
        delete computed_values;
    }

    void ComputeUpTo(int index) {
        if (is_finite && index >= finite_length)
            throw IndexOutOfRangeException("Index exceeds finite sequence length");
        
        int current_len = computed_values->GetLength();
        if (index < current_len) return;
        
        for (int i = current_len; i <= index; ++i) {
            T val = generator->Generate(i);
            computed_values->Append(val);
        }
    }

    const T& GetFirst() const override {
        if (computed_values->GetLength() == 0) {
            if (is_finite && finite_length == 0)
                throw IndexOutOfRangeException("Empty sequence");
            const_cast<LazySequence*>(this)->ComputeUpTo(0);
        }
        return computed_values->GetFirst();
    }

    const T& GetLast() const override {
        if (is_finite) {
            if (finite_length == 0)
                throw IndexOutOfRangeException("Empty sequence");
            const_cast<LazySequence*>(this)->ComputeUpTo(finite_length - 1);
            return computed_values->GetLast();
        } else {
            throw IndexOutOfRangeException("Cannot get last element of infinite lazy sequence");
        }
    }

    const T& Get(int index) const override {
        const_cast<LazySequence*>(this)->ComputeUpTo(index);
        if (index >= computed_values->GetLength())
            throw IndexOutOfRangeException("Index out of range");
        return computed_values->Get(index);
    }

    int GetLength() const override {
        if (is_finite)
            return finite_length;
        else
            return computed_values->GetLength();
    }

    void Reduce(T (*func)(const T&, const T&), const T& init, T* result) const override {
        T acc = init;
        for (int i = 0; i < GetLength(); i++) {
            acc = func(acc, Get(i));
        }
        *result = acc;
    }
    
    Option<T> TryGetFirst(bool (*pred)(const T&) = nullptr) const override {
        for (int i = 0; i < GetLength(); i++) {
            const T& val = Get(i);
            if (!pred || pred(val)) {
                return Option<T>(val);
            }
        }
        return Option<T>::None();
    }
    
    Option<T> TryGetLast(bool (*pred)(const T&) = nullptr) const override {
        for (int i = GetLength() - 1; i >= 0; i--) {
            const T& val = Get(i);
            if (!pred || pred(val)) {
                return Option<T>(val);
            }
        }
        return Option<T>::None();
    }
    
    const T& operator[](int index) const override {
        return Get(index);
    }
    
    class LazyEnumerator : public IEnumerator<T> {
    private:
        const LazySequence<T>& seq;
        int currentIndex;
    public:
        LazyEnumerator(const LazySequence<T>& s) : seq(s), currentIndex(-1) {}
        bool HasNext() override {
            if (currentIndex + 1 < seq.GetLength()) {
                currentIndex++;
                return true;
            }
            return false;
        }
        const T& GetCurrent() const override {
            if (currentIndex < 0 || currentIndex >= seq.GetLength()) {
                throw IndexOutOfRangeException("Enumerator not positioned");
            }
            return seq.Get(currentIndex);
        }
        void Reset() override { currentIndex = -1; }
    };
    
    IEnumerator<T>* GetEnumerator() const override {
        return new LazyEnumerator(*this);
    }


    Sequence<T>* GetSubsequence(int start, int end) const override {
        if (start < 0 || end < start)
            throw InvalidArgumentException("Invalid subsequence range");
        
        if (is_finite && end >= finite_length)
            throw InvalidArgumentException("End index exceeds sequence length");
        
        const_cast<LazySequence*>(this)->ComputeUpTo(end);
        
        MutableArraySequence<T>* sub = new MutableArraySequence<T>();
        for (int i = start; i <= end && i < computed_values->GetLength(); ++i)
            sub->Append(computed_values->Get(i));
        
        return sub;
    }

    Sequence<T>* Map(T (*func)(const T&)) const override {
        IGenerator<T>* new_gen = new MapGenerator<T>(generator, func);
        LazySequence<T>* new_seq;
        if (is_finite) new_seq = new LazySequence<T>(new_gen, finite_length);
        else new_seq = new LazySequence<T>(new_gen);
        
        for (int i = 0; i < computed_values->GetLength(); ++i)
            new_seq->computed_values->Append(func(computed_values->Get(i)));
        
        return new_seq;
    }

    Sequence<T>* Where(bool (*predicate)(const T&)) const override {
        IGenerator<T>* new_gen = new WhereGenerator<T>(generator, predicate);
        LazySequence<T>* new_seq = new LazySequence<T>(new_gen);
    
        for (int i = 0; i < computed_values->GetLength(); ++i) {
            T val = computed_values->Get(i);
            if (predicate(val))
                new_seq->computed_values->Append(val);
        }
        
        return new_seq;
    }
};


#endif 
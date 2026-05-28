#ifndef _IMMUTABLE_ARRAY_SEQUANCE_TPP_
#define _IMMUTABLE_ARRAY_SEQUANCE_TPP_


template <typename T>
Sequence<T>* ImmutableArraySequence<T>::Instance() {
    try {
        return new ImmutableArraySequence<T>(*this);
    } catch (...) {
        throw MemoryAllocationException("Failed to create changed array sequence");
    }
}

template <typename T>
Sequence<T>* ImmutableArraySequence<T>::CreateSequence(const DynamicArray<T> &arr) const {
    try {
        return new ImmutableArraySequence<T>(arr);
    } catch (...) {
        throw MemoryAllocationException("Failed to create immutable array sequence");
    }
}

template <typename T>
ImmutableArraySequence<T>::ImmutableArraySequence() 
    : ArraySequence<T>() {}

template <typename T>
ImmutableArraySequence<T>::ImmutableArraySequence(const T* items, int count) 
    : ArraySequence<T>(items, count) {}

template <typename T>
ImmutableArraySequence<T>::ImmutableArraySequence(int size) 
    : ArraySequence<T>(size) {}

template <typename T>
ImmutableArraySequence<T>::ImmutableArraySequence(const DynamicArray<T> &arr) 
    : ArraySequence<T>(arr) {}

template <typename T>
ImmutableArraySequence<T>::ImmutableArraySequence(const ArraySequence<T> &other) 
    : ArraySequence<T>(other) {}

#endif /* _MUTABLE_ARRAY_SEQUANCE_TPP_ */
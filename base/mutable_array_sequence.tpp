#ifndef _MUTABLE_ARRAY_SEQUANCE_TPP_
#define _MUTABLE_ARRAY_SEQUANCE_TPP_

template <typename T>
Sequence<T>* MutableArraySequence<T>::Instance() {
    return this;
}

template <typename T>
Sequence<T>* MutableArraySequence<T>::CreateSequence(const DynamicArray<T> &arr) const {
    try {
        return new MutableArraySequence<T>(arr);
    } catch (...) {
        throw MemoryAllocationException("Failed to create mutable array sequence");
    }
}

template <typename T>
MutableArraySequence<T>::MutableArraySequence() 
    : ArraySequence<T>() {}

template <typename T>
MutableArraySequence<T>::MutableArraySequence(const T* items, int count) 
    : ArraySequence<T>(items, count) {}

template <typename T>
MutableArraySequence<T>::MutableArraySequence(int size) 
    : ArraySequence<T>(size) {}

template <typename T>
MutableArraySequence<T>::MutableArraySequence(const DynamicArray<T> &arr) 
    : ArraySequence<T>(arr) {}

template <typename T>
MutableArraySequence<T>::MutableArraySequence(const ArraySequence<T> &other) 
    : ArraySequence<T>(other) {}

#endif /* _MUTABLE_ARRAY_SEQUANCE_TPP_ */
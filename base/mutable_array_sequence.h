#ifndef _MUTABLE_ARRAY_SEQUENCE_H_
#define _MUTABLE_ARRAY_SEQUENCE_H_

#include "array_sequence.h"

template <typename T>
class MutableArraySequence : public ArraySequence<T> {
protected:
    virtual Sequence<T>* Instance() override;
    virtual Sequence<T>* CreateSequence(const DynamicArray<T> &arr) const override;

public:
    MutableArraySequence();
    MutableArraySequence(const T* items, int count);
    explicit MutableArraySequence(int size);
    MutableArraySequence(const DynamicArray<T> &arr);
    MutableArraySequence(const ArraySequence<T> &other);
};

#include "mutable_array_sequence.tpp"

#endif // _MUTABLE_ARRAY_SEQUENCE_H_
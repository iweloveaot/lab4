#ifndef _IMMUTABLE_ARRAY_SEQUENCE_H_
#define _IMMUTABLE_ARRAY_SEQUENCE_H_

#include "array_sequence.h"

template <typename T>
class ImmutableArraySequence : public ArraySequence<T> {
protected:
    virtual Sequence<T>* Instance() override;
    virtual Sequence<T>* CreateSequence(const DynamicArray<T> &arr) const override;

public:
    ImmutableArraySequence();
    ImmutableArraySequence(const T* items, int count);
    explicit ImmutableArraySequence(int size);
    ImmutableArraySequence(const DynamicArray<T> &arr);
    ImmutableArraySequence(const ArraySequence<T> &other);
};

#include "immutable_array_sequence.tpp"

#endif // _IMMUTABLE_ARRAY_SEQUENCE_H_
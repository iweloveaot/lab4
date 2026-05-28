#ifndef _DYNAMIC_ARRAY_H_
#define _DYNAMIC_ARRAY_H_

#include "exceptions.h"

template <class T>
T* allocateMemory(int size); 

template <typename T>
class DynamicArray {
private:

    T* data;
    int size;
    int capacity;

    void ensureCapacity(int newSize);

public:
    DynamicArray();
    DynamicArray(const T *items, int count);
    explicit DynamicArray(int size);
    DynamicArray(const DynamicArray<T>& other);
    ~DynamicArray();

    const T& Get(int index) const;
    void Set(int index, T value);
    int GetSize() const;
    void Resize(int newSize); 

    DynamicArray<T>& operator=(const DynamicArray<T> &other);
};

#include "dynamic_array.tpp"

#endif // _DYNAMIC_ARRAY_H_
#ifndef _DYNAMIC_ARRAY_TPP_
#define _DYNAMIC_ARRAY_TPP_

template <class T>
T* allocateMemory(int size) {
    if (size == 0) {
        return nullptr;
    }
    T* ptr = nullptr;
    try {
        ptr = new T[size]();
    } catch (...) {
        throw MemoryAllocationException("Failed to create dynamic array");
    }
    return ptr;
}


// Приватный метод
template <typename T>
void DynamicArray<T>::ensureCapacity(int newSize) {
    if (newSize <= capacity) 
        return;
    int newCap = capacity * 2;
    if (newCap < newSize) newCap = newSize;
    T* newData = allocateMemory<T>(newCap);
    for (int i = 0; i < size; ++i)
        newData[i] = data[i];
    delete[] data;
    data = newData;
    capacity = newCap;
}

// Конструкторы и деструктор
template <typename T>
DynamicArray<T>::DynamicArray() : data(nullptr), size(0), capacity(0) {}

template <typename T>
DynamicArray<T>::DynamicArray(const T* items, int count) 
    : size(count), capacity(count) {
    if (count < 0) 
        throw InvalidArgumentException("Negative count for dynamic array");
    if (items == nullptr && count > 0)
        throw NullReferenceException("Null pointer for dynamic array");
    data = allocateMemory<T>(capacity);
    for (int i = 0; i < count; i++)
        data[i] = items[i];
}

template <typename T>
DynamicArray<T>::DynamicArray(int size) : size(size), capacity(size) {
    if (size < 0) throw InvalidArgumentException("Negative size for dynamic array");
    data = allocateMemory<T>(capacity);
    for (int i = 0; i < size; i++)
        data[i] = T();
}

template <typename T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other) 
    : size(other.size), capacity(other.capacity) {
    if (other.data == nullptr && other.size > 0)
        throw NullReferenceException("Null pointer when copying a dynamic array");
    data = allocateMemory<T>(capacity);
    for (int i = 0; i < size; i++)
        data[i] = other.data[i];
}

template <typename T>
DynamicArray<T>::~DynamicArray() {
    delete[] data;
}

// Публичные методы
template <typename T>
const T& DynamicArray<T>::Get(int index) const {
    if (index < 0 || index >= size)
        throw IndexOutOfRangeException("Index out of range in DynamicArray::Get");
    return data[index];
}

template <typename T>
void DynamicArray<T>::Set(int index, T value) {
    if (index < 0 || index >= size)
        throw IndexOutOfRangeException("Index out of range in DynamicArray::Set");
    data[index] = value;
}

template <typename T>
int DynamicArray<T>::GetSize() const { 
    return size; 
}

template <typename T>
void DynamicArray<T>::Resize(int newSize) {
    if (newSize < 0) 
        throw InvalidArgumentException("Negative new size for dynamic array");
    if (newSize == size)    
        return;
    ensureCapacity(newSize);
    if (newSize < size) {
        T* newData = allocateMemory<T>(capacity);
        for (int i = 0; i < newSize; i++)
            newData[i] = data[i];
        delete[] data;
        data = newData;
    }
    if (newSize > size) {
        for (int i = size; i < newSize; i++)
            data[i] = T();
    }
    size = newSize;
}

// Оператор присваивания
template <typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other) {
    if (other.data == nullptr && other.size > 0)
        throw NullReferenceException("Null pointer when copying a dynamic array");
    if (this != &other) {
        delete[] data;
        size = other.size;
        capacity = other.capacity;
        data = allocateMemory<T>(capacity);
        for (int i = 0; i < size; ++i)
            data[i] = other.data[i];
    }
    return *this;
}

#endif // _DYNAMIC_ARRAY_TPP_
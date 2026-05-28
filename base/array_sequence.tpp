#ifndef _ARRAY_SEQUANCE_TPP_
#define _ARRAY_SEQUANCE_TPP_


template <typename T>
Sequence<T>* ArraySequence<T>::AppendImplict(const T &item) {
    int size = array.GetSize();
    array.Resize(size+1);
    array.Set(size, item);
    return this;
}

template <typename T>
Sequence<T>* ArraySequence<T>::PrependImplict(const T &item) {
    int size = array.GetSize();
    DynamicArray<T> tmp_arr(size+1);
    for (int i = 1; i <= array.GetSize(); i++) {
        tmp_arr.Set(i, array.Get(i-1));
    }
    tmp_arr.Set(0, item);
    array = tmp_arr;
    return this;
}

template <typename T>
Sequence<T>* ArraySequence<T>::InsertAtImplict(const T &item, int index) {
    if (index < 0 || index > array.GetSize())
        throw IndexOutOfRangeException("Index out of range in Sequence::InsertAt");
    
    if (index == array.GetSize())
        AppendImplict(item);
    else if (index == 0)
        PrependImplict(item);
    else {
        DynamicArray<T> tmp_arr(array.GetSize()+1);
        for (int i = 0; i < index; i++)
            tmp_arr.Set(i, array.Get(i));
        tmp_arr.Set(index, item);
        for (int i = index; i < array.GetSize(); i++)
            tmp_arr.Set(i+1, array.Get(i));
        array = tmp_arr;
    }
    return this;
}

template <typename T>
Sequence<T>* ArraySequence<T>::RemoveAtImplict(int index) {
    if (index < 0 || index > array.GetSize())
        throw IndexOutOfRangeException("Index out of range in Sequence::RemoveAt");

    DynamicArray<T> tmp_arr(array.GetSize()-1);
        for (int i = 0; i < index; i++)
            tmp_arr.Set(i, array.Get(i));
        for (int i = index+1; i < array.GetSize(); i++)
            tmp_arr.Set(i-1, array.Get(i));
        array = tmp_arr;

    return this;
}

template <typename T>
Sequence<T>* ArraySequence<T>::ConcatImplict(const Sequence<T>* other) {
    if (other == nullptr)
        throw NullReferenceException("Null pointer in ArraySequence::Concat");  
    for (int i = 0; i < other->GetLength(); i++) {
        AppendImplict(other->Get(i));
    }
    return this;
}

template <typename T>
ArraySequence<T>::ArraySequence() : array(DynamicArray<T>()) {}

template <typename T>
ArraySequence<T>::ArraySequence(const T* items, int count) 
    : array(DynamicArray<T>(items, count)) {}

template <typename T>
ArraySequence<T>::ArraySequence(int size) 
    : array(DynamicArray<T>(size)) {}

template <typename T>
ArraySequence<T>::ArraySequence(const DynamicArray<T> &arr) 
    : array(DynamicArray<T>(arr)) {}

template <typename T>
ArraySequence<T>::ArraySequence(const ArraySequence<T> &other) 
    : array(DynamicArray<T>(other.array)) {}

template <typename T>
ArraySequence<T>::~ArraySequence() {}

template <typename T>
ArraySequence<T>& ArraySequence<T>::operator=(const ArraySequence<T>& other) {
    if (this != &other) {
        array = other.array;    
    }
    return *this;
}

template <typename T>
const T& ArraySequence<T>::GetFirst() const {
    if (array.GetSize() == 0)
        throw EmptyContainerException("Array sequence is empty");
    return array.Get(0);
}

template <typename T>
const T& ArraySequence<T>::GetLast() const {
    if (array.GetSize() == 0)
        throw EmptyContainerException("Array sequence is empty");
    return array.Get(array.GetSize()-1);
}

template <typename T>
const T& ArraySequence<T>::Get(int index) const {
    return array.Get(index);
}

template <typename T>
int ArraySequence<T>::GetLength() const {
    return array.GetSize();
}

template <typename T>
Sequence<T>* ArraySequence<T>::GetSubsequence(int startIndex, int endIndex) const {
    if (startIndex < 0 || endIndex >= array.GetSize() || startIndex > endIndex)
        throw IndexOutOfRangeException("Invalid subsequence indexes");
    
    DynamicArray<T> tmp_array(endIndex - startIndex + 1);
    int tmp_ind = 0;
    for (int i = startIndex; i <= endIndex; i++) {
        tmp_array.Set(tmp_ind, array.Get(i));
        tmp_ind++;
    }
    return CreateSequence(tmp_array);
}

template <typename T>
Sequence<T>* ArraySequence<T>::Map(T (*func)(const T&)) const {
    int size = array.GetSize();
    DynamicArray<T> mapped(size);
    for (int i = 0; i < size; i++) {
        T val = array.Get(i);
        mapped.Set(i, func(val));
    }
    return CreateSequence(mapped);
}

template <typename T>
Sequence<T>* ArraySequence<T>::Where(bool (*pred)(const T&)) const {
    DynamicArray<T> filtered;
    for (int i = 0; i < array.GetSize(); i++) {
        T val = array.Get(i);
        if (pred(val)) {
            filtered.Resize(filtered.GetSize() + 1);
            filtered.Set(filtered.GetSize() - 1, val);
        }
    }
    return CreateSequence(filtered);
}

template <typename T>
void ArraySequence<T>::Reduce(T (*func)(const T&, const T&), const T &init, T *result) const {
    T reduced = init;
    for (int i = 0; i < array.GetSize(); i++)
        reduced = func(reduced, array.Get(i));
    *result = reduced;
}

template <typename T>
Option<T> ArraySequence<T>::TryGetFirst(bool (*pred)(const T&)) const {
    for (int i = 0; i < array.GetSize(); i++) {
        const T& val = Get(i);
        if (!pred || pred(val))
            return Option<T>(val);
    }
    return Option<T>::None();
}

template <typename T>
Option<T> ArraySequence<T>::TryGetLast(bool (*pred)(const T&)) const {
    for (int i = array.GetSize() - 1; i >= 0; i--) {
        const T& val = Get(i);
        if (!pred || pred(val))
            return Option<T>(val);
    }
    return Option<T>::None();
}

template <typename T>
ArraySequence<T>::ArrayEnumerator::ArrayEnumerator(const ArraySequence<T>& s) 
    : arr_seq(s), currentIndex(-1) {}

template <typename T>
bool ArraySequence<T>::ArrayEnumerator::HasNext() {
    if (currentIndex + 1 < arr_seq.GetLength()) {
        ++currentIndex;
        return true;
    }
    return false;
}

template <typename T>
const T& ArraySequence<T>::ArrayEnumerator::GetCurrent() const {
    if (currentIndex < 0 || currentIndex >= arr_seq.GetLength())
        throw IndexOutOfRangeException("Enumerator not positioned");
    return arr_seq.Get(currentIndex);
}

template <typename T>
void ArraySequence<T>::ArrayEnumerator::Reset() { 
    currentIndex = -1; 
}

template <typename T>
IEnumerator<T>* ArraySequence<T>::GetEnumerator() const {
    return new ArrayEnumerator(*this);
}
    
#endif /* _ARRAY_SEQUANCE_TPP_ */
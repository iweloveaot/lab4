#ifndef _SEQUENCE_TPP_
#define _SEQUENCE_TPP_

template <typename T>
Sequence<T>* Sequence<T>::Append(const T &item) {
    return Instance()->AppendImplict(item);
}

template <typename T>
Sequence<T>* Sequence<T>::Prepend(const T &item) {
    return Instance()->PrependImplict(item);
}

template <typename T>
Sequence<T>* Sequence<T>::InsertAt(const T &item, int index) {
    return Instance()->InsertAtImplict(item, index);
}

template <typename T>
Sequence<T>* Sequence<T>::RemoveAt(int index) {
    return Instance()->RemoveAtImplict(index);
}

template <typename T>
Sequence<T>* Sequence<T>::Concat(const Sequence<T> *other) {
    return Instance()->ConcatImplict(other);
}

template <typename T>
const T& Sequence<T>::operator[](int index) const {
    return Get(index);
}

template <typename T>
Sequence<T>::~Sequence() {}

#endif
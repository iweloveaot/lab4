#ifndef _OPTION_TPP_
#define _OPTION_TPP_

template<typename T>
Option<T>::Option() : value(nullptr), hasValue(false) {}

template<typename T>
Option<T>::Option(const T& val) : hasValue(true) {
    try {
        value = new T(val);
    } catch (...) {
        throw MemoryAllocationException("Failed to allocate Option value");
    }
}

template<typename T>
Option<T>::Option(const Option& other) : hasValue(other.hasValue), value(nullptr) {
    if (hasValue) {
        try {
            value = new T(*other.value);
        } catch (...) {
            throw MemoryAllocationException("Failed to allocate Option value in copy");
        }
    }
}

template<typename T>
Option<T>::~Option() {
    delete value;
}

template<typename T>
Option<T>& Option<T>::operator=(const Option& other) {
    if (this != &other) {
        delete value;
        hasValue = other.hasValue;
        if (hasValue) {
            try {
                value = new T(*other.value);
            } catch (...) {
                value = nullptr;
                hasValue = false;
                throw MemoryAllocationException("Failed to allocate Option value in assignment");
            }
        } else {
            value = nullptr;
        }
    }
    return *this;
}

template<typename T>
bool Option<T>::HasValue() const {
    return hasValue;
}

template<typename T>
const T& Option<T>::GetValue() const {
    if (!hasValue)
        throw EmptyContainerException("Option has no value");
    return *value;
}

template<typename T>
T Option<T>::GetValueOrDefault(const T& defaultValue) const {
    return hasValue ? *value : defaultValue;
}

template<typename T>
Option<T> Option<T>::None() {
    return Option<T>();
}

template<typename T>
Option<T> Option<T>::Some(const T& val) {
    return Option<T>(val);
}

#endif /* _OPTION_TPP_ */
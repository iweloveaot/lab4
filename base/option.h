#ifndef _OPTION_H_
#define _OPTION_H_

#include "exceptions.h"

template<typename T>
class Option {
private:
    T* value;
    bool hasValue;

public:
    Option();
    Option(const T& val);
    Option(const Option& other);

    ~Option();

    Option& operator=(const Option& other);

    bool HasValue() const;
    const T& GetValue() const;
    T GetValueOrDefault(const T& defaultValue) const;

    static Option<T> None();
    static Option<T> Some(const T& val);
};

#include "option.tpp"

#endif // _OPTION_H_
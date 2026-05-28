#ifndef _ITERABLE_H_
#define _ITERABLE_H_

#include "exceptions.h"

template<typename T>
class IEnumerator {
public:
    virtual ~IEnumerator() {}
    
    virtual bool HasNext() = 0;
    virtual void Reset() = 0;
    virtual const T& GetCurrent() const = 0;
};

template<typename T>
class IEnumerable {
public:
    virtual ~IEnumerable() {}
    virtual IEnumerator<T>* GetEnumerator() const = 0;
};

#endif /* _ITERABLE_H_ */
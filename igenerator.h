#ifndef IGENERATOR_H
#define IGENERATOR_H

template<typename T>
class IGenerator {
public:
    virtual ~IGenerator() {}
    virtual T Generate(int localIndex) const = 0;
    virtual IGenerator<T>* Clone() const = 0;
};

#endif
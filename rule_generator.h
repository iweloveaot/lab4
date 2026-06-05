#ifndef RULE_GENERATOR_H
#define RULE_GENERATOR_H

#include "IGenerator.h"

template<typename T>
class RuleGenerator : public IGenerator<T> {
private:
    T (*ruleFunc)(int);
public:
    RuleGenerator(T (*func)(int)) : ruleFunc(func) {}
    
    T Generate(int localIndex) const override { return ruleFunc(localIndex); }
    IGenerator<T>* Clone() const override { return new RuleGenerator<T>(ruleFunc); }
};

#endif
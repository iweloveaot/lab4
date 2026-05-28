#include "markov_machine.h"
#include <iostream>

int main() {

    char *pat = "aaa";
    char *perl = "3A";
    MarkovMachine mm;
    mm.AddRule(pat, perl);
    std::cout << mm.GetRulePattern(0) << " -> " << mm.GetRuleReplacement(0) << std::endl;
    mm.SetInput("bbbaaabbbaaa");
    MutableArraySequence<char> *inp = mm.GetInput();
    for (int i=0; i<inp->GetLength(); i++) {
        std::cout << inp->Get(i);
    }
    std::cout << std::endl;
    mm.Execute();
    inp = mm.GetInput();
    for (int i=0; i<inp->GetLength(); i++) {
        std::cout << inp->Get(i);
    }
    std::cout << std::endl << mm.IsHalted() << " " << mm.GetSteps();

    
}
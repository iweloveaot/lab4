#ifndef _MARKOV_MACHINE_H_
#define _MARKOV_MACHINE_H_

#include <iostream>
#include "base/mutable_array_sequence.h"
#include "func_for_mm.h"


class MarkovRule {
private:
    char* pattern;
    char* replacement;
    bool isFinal;
    

public:
    MarkovRule(const char* pat, const char* repl, bool final = false) {
        pattern = StrDup(pat);
        replacement = StrDup(repl);
        isFinal = final;
    }
    
    MarkovRule(const MarkovRule& other) {
        pattern = StrDup(other.pattern);
        replacement = StrDup(other.replacement);
        isFinal = other.isFinal;
    }
    
    ~MarkovRule() {
        delete[] pattern;
        delete[] replacement;
    }
    
    const char* GetPattern() const { return pattern; }
    const char* GetReplacement() const { return replacement; }
    bool IsFinal() const { return isFinal; }
    
    int FindInSequence(const MutableArraySequence<char>& seq, int startPos = 0) const {
        int seqLen = seq.GetLength();
        int patLen = StrLen(pattern);
        if (patLen == 0) return startPos;
        if (startPos < 0) startPos = 0;
        
        for (int i = startPos; i <= seqLen - patLen; i++) {
            bool match = true;
            for (int j = 0; j < patLen; j++) {
                if (seq.Get(i + j) != pattern[j]) {
                    match = false;
                    break;
                }
            }
            if (match) return i;
        }
        return -1;
    }
    
    void ApplyToSequence(MutableArraySequence<char>& seq, int pos) const {
        int patLen = StrLen(pattern);
        int replLen = StrLen(replacement);
        
        for (int i = 0; i < patLen; i++) {
            seq.RemoveAt(pos); 
        }
        
        for (int i = replLen - 1; i >= 0; i--) {
            seq.InsertAt(replacement[i], pos);
        }
    }
};


class MarkovMachine {
private:
    MutableArraySequence<char>* input;  
    DynamicArray<MarkovRule*> rules;
    int maxSteps;
    int currentStep;
    bool halted;
    
    MutableArraySequence<char>* StrToSequence(const char* str) {
        if (!str) return new MutableArraySequence<char>();
        int len = StrLen(str);
        MutableArraySequence<char>* seq = new MutableArraySequence<char>();
        for (int i = 0; i < len; i++) {
            seq->Append(str[i]);
        }
        return seq;
    }

public:
    MarkovMachine(int maxIter = 10000) : input(nullptr), maxSteps(maxIter), currentStep(0), halted(false) {}
    
    ~MarkovMachine() {
        delete input;
        for (int i = 0; i < rules.GetSize(); i++) {
            delete rules.Get(i);
        }
    }
    
    void AddRule(const char* pattern, const char* replacement, bool isFinal = false) {
        rules.Resize(rules.GetSize() + 1);
        rules.Set(rules.GetSize() - 1, new MarkovRule(pattern, replacement, isFinal));
    }
    
    void SetInput(const char* str) {
        delete input;
        input = StrToSequence(str);
        halted = false;
        currentStep = 0;
    }
    
    MutableArraySequence<char>* GetInput() const {
        if (!input) return nullptr;
        return input;
    }

    const char* GetRulePattern(int index) const {
        const char *res = rules.Get(index)->GetPattern();
        return res;
    }

    const char* GetRuleReplacement(int index) const {
        const char *res = rules.Get(index)->GetReplacement();
        return res;
    }
    
    bool Step() {
        if (!input || halted) return false;
        if (currentStep >= maxSteps) {
            halted = true;
            return false;
        }
        
        for (int r = 0; r < rules.GetSize(); r++) {
            MarkovRule* rule = rules.Get(r);
            int pos = rule->FindInSequence(*input);
            if (pos >= 0) {
                rule->ApplyToSequence(*input, pos);
                currentStep++;
                if (rule->IsFinal()) {
                    halted = true;
                }
                return true; 
            }
        }
        halted = true;
        return false;
    }
    
    bool Execute() {
        while (!halted) {
            if (!Step()) break;
        }
        return !halted || currentStep < maxSteps;
    }
    
    bool IsHalted() const { return halted; }
    int GetSteps() const { return currentStep; }
    int GetLength() const { return input ? input->GetLength() : 0; }
};

#endif // _MARKOV_MACHINE_H_


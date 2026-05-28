
// MarkovMachine.h
#ifndef MARKOV_MACHINE_H
#define MARKOV_MACHINE_H

#include <iostream>
#include "base/mutable_array_sequence.h"

// Вспомогательная функция для работы с C-строками без <string>
inline int StrLen(const char* str) {
    if (!str) return 0;
    int len = 0;
    while (str[len]) len++;
    return len;
}

inline bool StrEq(const char* a, const char* b) {
    if (!a && !b) return true;
    if (!a || !b) return false;
    int i = 0;
    while (a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] == b[i];
}

inline int StrFind(const char* text, const char* pattern, int startPos = 0) {
    if (!text || !pattern) return -1;
    int textLen = StrLen(text);
    int patLen = StrLen(pattern);
    if (patLen == 0) return startPos;
    if (startPos < 0) startPos = 0;
    
    for (int i = startPos; i <= textLen - patLen; i++) {
        bool match = true;
        for (int j = 0; j < patLen; j++) {
            if (text[i + j] != pattern[j]) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

inline char* StrDup(const char* src) {
    if (!src) return nullptr;
    int len = StrLen(src);
    char* dup = new char[len + 1];
    for (int i = 0; i <= len; i++) dup[i] = src[i];
    return dup;
}

// Правило Маркова: шаблон -> замена
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
    
    // Найти первое вхождение паттерна в последовательность символов
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
    
    // Применить замену в последовательности
    void ApplyToSequence(MutableArraySequence<char>& seq, int pos) const {
        int patLen = StrLen(pattern);
        int replLen = StrLen(replacement);
        
        // Удаляем паттерн
        for (int i = 0; i < patLen; i++) {
            seq.RemoveAt(pos);  // Предполагаем, что MutableArraySequence имеет RemoveAt
        }
        
        // Вставляем замену
        for (int i = replLen - 1; i >= 0; i--) {
            seq.InsertAt(replacement[i], pos);
        }
    }
};

// Машина Маркова: реализация нормальных алгоритмов Маркова
class MarkovMachine {
private:
    MutableArraySequence<char>* input;  // Текущее состояние как последовательность символов
    DynamicArray<MarkovRule*> rules;    // Список правил
    int maxSteps;
    int currentStep;
    bool halted;
    
    // Преобразовать C-строку в последовательность
    MutableArraySequence<char>* StrToSequence(const char* str) {
        if (!str) return new MutableArraySequence<char>();
        int len = StrLen(str);
        MutableArraySequence<char>* seq = new MutableArraySequence<char>(len);
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
    
    // Добавить правило
    void AddRule(const char* pattern, const char* replacement, bool isFinal = false) {
        rules.Resize(rules.GetSize() + 1);
        rules.Set(rules.GetSize() - 1, new MarkovRule(pattern, replacement, isFinal));
    }
    
    // Установить входную строку
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
    
    // Выполнить один шаг алгоритма
    bool Step() {
        if (!input || halted) return false;
        if (currentStep >= maxSteps) {
            halted = true;
            return false;
        }
        
        // Поиск первого применимого правила
        for (int r = 0; r < rules.GetSize(); r++) {
            MarkovRule* rule = rules.Get(r);
            int pos = rule->FindInSequence(*input);
            if (pos >= 0) {
                // Применяем правило
                rule->ApplyToSequence(*input, pos);
                currentStep++;
                // Если правило финальное — останавливаемся
                if (rule->IsFinal()) {
                    halted = true;
                }
                return true;  // Шаг выполнен
            }
        }
        // Ни одно правило не применимо
        halted = true;
        return false;
    }
    
    // Выполнить алгоритм до завершения
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

#endif // MARKOV_MACHINE_H


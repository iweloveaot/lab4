#ifndef _MARKOV_TESTS_
#define _MARKOV_TESTS_

#include <iostream>
#include <cstring>
#include <cassert>
#include "markov_machine.h"

using namespace std;

// ==========================================
// Простой тестовый фреймворк
// ==========================================

int testsPassed = 0;
int testsFailed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    cout << "Running " << #name << "... "; \
    try { \
        name(); \
        cout << "PASSED" << endl; \
        testsPassed++; \
    } catch (const exception& e) { \
        cout << "FAILED: " << e.what() << endl; \
        testsFailed++; \
    } catch (...) { \
        cout << "FAILED: unknown exception" << endl; \
        testsFailed++; \
    } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        throw runtime_error(string("Assertion failed: expected ") + to_string(expected) + \
                           " but got " + to_string(actual) + " at line " + to_string(__LINE__)); \
    } \
} while(0)

#define ASSERT_STREQ(expected, actual) do { \
    if (strcmp((expected), (actual)) != 0) { \
        throw runtime_error(string("Assertion failed: expected \"") + (expected) + \
                           "\" but got \"" + (actual) + "\" at line " + to_string(__LINE__)); \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        throw runtime_error(string("Assertion failed: condition is false at line ") + to_string(__LINE__)); \
    } \
} while(0)

#define ASSERT_FALSE(cond) do { \
    if ((cond)) { \
        throw runtime_error(string("Assertion failed: condition is true at line ") + to_string(__LINE__)); \
    } \
} while(0)

// ==========================================
// Утилита для получения строки из последовательности
// ==========================================

string SeqToStr(MutableArraySequence<char>* seq) {
    if (!seq) return "";
    string result;
    int len = seq->GetLength();
    for (int i = 0; i < len; i++) {
        result += seq->Get(i);
    }
    return result;
}

// Утилита для создания последовательности из строки
MutableArraySequence<char>* CreateSeqFromString(const char* str) {
    MutableArraySequence<char>* seq = new MutableArraySequence<char>();
    for (int i = 0; str[i] != '\0'; i++) {
        seq->Append(str[i]);
    }
    return seq;
}

// ==========================================
// Тесты для MarkovRule
// ==========================================

TEST(TestMarkovRule_Construction) {
    MarkovRule rule("abc", "def", false);
    ASSERT_STREQ("abc", rule.GetPattern());
    ASSERT_STREQ("def", rule.GetReplacement());
    ASSERT_FALSE(rule.IsFinal());
}

TEST(TestMarkovRule_FinalRule) {
    MarkovRule rule("x", "y", true);
    ASSERT_TRUE(rule.IsFinal());
}

TEST(TestMarkovRule_CopyConstructor) {
    MarkovRule rule1("hello", "world", true);
    MarkovRule rule2(rule1);
    ASSERT_STREQ("hello", rule2.GetPattern());
    ASSERT_STREQ("world", rule2.GetReplacement());
    ASSERT_TRUE(rule2.IsFinal());
}

TEST(TestMarkovRule_FindInSequence_Found) {
    MarkovRule rule("bc", "XY");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    int pos = rule.FindInSequence(seq);
    ASSERT_EQ(1, pos);
}

TEST(TestMarkovRule_FindInSequence_NotFound) {
    MarkovRule rule("xyz", "XY");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    int pos = rule.FindInSequence(seq);
    ASSERT_EQ(-1, pos);
}

TEST(TestMarkovRule_FindInSequence_AtStart) {
    MarkovRule rule("ab", "XY");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    int pos = rule.FindInSequence(seq);
    ASSERT_EQ(0, pos);
}

TEST(TestMarkovRule_FindInSequence_AtEnd) {
    MarkovRule rule("ef", "XY");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    int pos = rule.FindInSequence(seq);
    ASSERT_EQ(4, pos);
}

TEST(TestMarkovRule_ApplyToSequence) {
    MarkovRule rule("bc", "XYZ");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    rule.ApplyToSequence(seq, 1);
    string result = SeqToStr(&seq);
    ASSERT_STREQ("aXYZdef", result.c_str());
}

TEST(TestMarkovRule_ApplyToSequence_EmptyReplacement) {
    MarkovRule rule("bc", "");
    const char* str = "abcdef";
    MutableArraySequence<char> seq;
    for (int i = 0; str[i] != '\0'; i++) {
        seq.Append(str[i]);
    }
    rule.ApplyToSequence(seq, 1);
    string result = SeqToStr(&seq);
    ASSERT_STREQ("adef", result.c_str());
}

// ==========================================
// Тесты для MarkovMachine
// ==========================================

TEST(TestMarkovMachine_Construction) {
    MarkovMachine mm;
    ASSERT_FALSE(mm.IsHalted());
    ASSERT_EQ(0, mm.GetSteps());
    ASSERT_EQ(0, mm.GetLength());
}

TEST(TestMarkovMachine_SetInput) {
    MarkovMachine mm;
    mm.SetInput("hello");
    ASSERT_EQ(5, mm.GetLength());
    ASSERT_FALSE(mm.IsHalted());
}

TEST(TestMarkovMachine_AddRule) {
    MarkovMachine mm;
    mm.AddRule("a", "b", false);
    ASSERT_STREQ("a", mm.GetRulePattern(0));
    ASSERT_STREQ("b", mm.GetRuleReplacement(0));
}

TEST(TestMarkovMachine_SingleStep) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("a", "X", false);
    bool result = mm.Step();
    ASSERT_TRUE(result);
    ASSERT_EQ(1, mm.GetSteps());
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("Xbc", str.c_str());
}

TEST(TestMarkovMachine_NoMatchingRule) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("z", "X", false);
    bool result = mm.Step();
    ASSERT_FALSE(result);
    ASSERT_TRUE(mm.IsHalted());
}

TEST(TestMarkovMachine_FinalRuleHalts) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("a", "X", true);
    bool result = mm.Step();
    ASSERT_TRUE(result);
    ASSERT_TRUE(mm.IsHalted());
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("Xbc", str.c_str());
}

TEST(TestMarkovMachine_Execute_Simple) {
    MarkovMachine mm;
    mm.SetInput("aaa");
    mm.AddRule("a", "b", false);
    mm.Execute();
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("bbb", str.c_str());
    ASSERT_TRUE(mm.IsHalted());
}

TEST(TestMarkovMachine_Execute_WithFinal) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("a", "X", false);
    mm.AddRule("b", "Y", true);
    mm.AddRule("c", "Z", false);
    mm.Execute();
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("XYc", str.c_str());
    ASSERT_TRUE(mm.IsHalted());
}

TEST(TestMarkovMachine_Execute_Deletion) {
    MarkovMachine mm;
    mm.SetInput("a0b00c0");
    mm.AddRule("0", "", false);
    mm.Execute();
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("abc", str.c_str());
}

TEST(TestMarkovMachine_Execute_UnaryAddition) {
    MarkovMachine mm;
    mm.SetInput("111+11");
    mm.AddRule("1+1", "11", false);
    mm.AddRule("+", "", true);
    mm.Execute();
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("11111", str.c_str());
}

TEST(TestMarkovMachine_MaxSteps) {
    MarkovMachine mm(100);
    mm.SetInput("a");
    mm.AddRule("a", "b", false);
    mm.AddRule("b", "a", false);
    mm.Execute();
    ASSERT_TRUE(mm.IsHalted());
    ASSERT_TRUE(mm.GetSteps() <= 100);
}

TEST(TestMarkovMachine_EmptyInput) {
    MarkovMachine mm;
    mm.SetInput("");
    mm.AddRule("a", "b", false);
    bool result = mm.Step();
    ASSERT_FALSE(result);
    ASSERT_TRUE(mm.IsHalted());
}

TEST(TestMarkovMachine_EmptyPattern) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("", "X", false);
    bool result = mm.Step();
    ASSERT_TRUE(result);
    ASSERT_EQ(1, mm.GetSteps());
}

TEST(TestMarkovMachine_MultipleRules_FirstMatch) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("a", "X", false);
    mm.AddRule("a", "Y", false);
    mm.Step();
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("Xbc", str.c_str());
}

TEST(TestMarkovMachine_SetInputResetsState) {
    MarkovMachine mm;
    mm.SetInput("abc");
    mm.AddRule("a", "X", true);
    mm.Step();
    ASSERT_TRUE(mm.IsHalted());
    
    mm.SetInput("def");
    ASSERT_FALSE(mm.IsHalted());
    ASSERT_EQ(0, mm.GetSteps());
}

TEST(TestMarkovMachine_GetInput) {
    MarkovMachine mm;
    ASSERT_TRUE(mm.GetInput() == nullptr);
    mm.SetInput("test");
    ASSERT_TRUE(mm.GetInput() != nullptr);
    string str = SeqToStr(mm.GetInput());
    ASSERT_STREQ("test", str.c_str());
}

// ==========================================
// Главная функция
// ==========================================

void RunMarkovMachineTests() {
    cout << "========================================" << endl;
    cout << "  Markov Machine — Unit Tests" << endl;
    cout << "========================================" << endl << endl;

    // MarkovRule tests
    RUN_TEST(TestMarkovRule_Construction);
    RUN_TEST(TestMarkovRule_FinalRule);
    RUN_TEST(TestMarkovRule_CopyConstructor);
    RUN_TEST(TestMarkovRule_FindInSequence_Found);
    RUN_TEST(TestMarkovRule_FindInSequence_NotFound);
    RUN_TEST(TestMarkovRule_FindInSequence_AtStart);
    RUN_TEST(TestMarkovRule_FindInSequence_AtEnd);
    RUN_TEST(TestMarkovRule_ApplyToSequence);
    RUN_TEST(TestMarkovRule_ApplyToSequence_EmptyReplacement);

    // MarkovMachine tests
    RUN_TEST(TestMarkovMachine_Construction);
    RUN_TEST(TestMarkovMachine_SetInput);
    RUN_TEST(TestMarkovMachine_AddRule);
    RUN_TEST(TestMarkovMachine_SingleStep);
    RUN_TEST(TestMarkovMachine_NoMatchingRule);
    RUN_TEST(TestMarkovMachine_FinalRuleHalts);
    RUN_TEST(TestMarkovMachine_Execute_Simple);
    RUN_TEST(TestMarkovMachine_Execute_WithFinal);
    RUN_TEST(TestMarkovMachine_Execute_Deletion);
    RUN_TEST(TestMarkovMachine_Execute_UnaryAddition);
    RUN_TEST(TestMarkovMachine_MaxSteps);
    RUN_TEST(TestMarkovMachine_EmptyInput);
    RUN_TEST(TestMarkovMachine_EmptyPattern);
    RUN_TEST(TestMarkovMachine_MultipleRules_FirstMatch);
    RUN_TEST(TestMarkovMachine_SetInputResetsState);
    RUN_TEST(TestMarkovMachine_GetInput);

    cout << endl << "========================================" << endl;
    cout << "  Results: " << testsPassed << " passed, " << testsFailed << " failed" << endl;
    cout << "========================================" << endl;
}

#endif
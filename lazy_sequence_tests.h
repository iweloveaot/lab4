#ifndef _LAZY_TESTS_
#define _LAZY_TESTS_

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include "rule_generator.h"
#include "lazy_sequence.h"
#include "ordinal_index_parser.h"
#include "base/exceptions.h"

int tests_passed = 0;
int tests_failed = 0;

#define TEST_CASE(name) void test_##name()
#define RUN_TEST(name) \
    try { \
        std::cout << "[RUN] " << #name << "... "; \
        test_##name(); \
        std::cout << "\033[32mPASSED\033[0m\n"; \
        tests_passed++; \
    } catch (const std::exception& e) { \
        std::cout << "\033[31mFAILED\033[0m\n"; \
        std::cerr << "  Error: " << e.what() << "\n"; \
        tests_failed++; \
    }

#define ASSERT_EQ(actual, expected) \
    if ((actual) != (expected)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #actual + " != " + #expected + \
                                 " (got " + std::to_string(actual) + ", expected " + std::to_string(expected) + ")"); \
    }

#define ASSERT_EQ_STR(actual, expected) \
    if (std::string(actual) != std::string(expected)) { \
        throw std::runtime_error(std::string("Assertion failed: ") + #actual + " != " + #expected); \
    }

#define ASSERT_THROW(expr, exc_type) \
    try { \
        expr; \
        throw std::runtime_error("Expected exception " #exc_type " was not thrown"); \
    } catch (const exc_type&) { \
         \
    } catch (...) { \
        throw std::runtime_error("Wrong exception type thrown for " #expr); \
    }


int TestSquare(const int &i) { return (i + 1) * (i + 1); }
int TestCube(const int &i)   { return (i + 1) * (i + 1) * (i + 1); }
int TestLinear(const int &i) { return i * 10; }


TEST_CASE(BasicGenerationAndCaching) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> seq(&gen);

    ASSERT_EQ(seq.Get(0), 1);
    ASSERT_EQ(seq.Get(4), 25);
    ASSERT_EQ(seq.Get(9), 100);

    ASSERT_EQ(seq.CachedCount(), 3);
    ASSERT_EQ(seq.IsCached(OrdinalIndexParser::Parse("4")), true);
}

TEST_CASE(ImmutablePrependAndAppend) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> base(&gen);

    LazySequence<int>* seq1 = base.AppendLazy(99);
    LazySequence<int>* seq2 = seq1->PrependLazy(100);

    ASSERT_EQ(base.Get(0), 1);
    ASSERT_EQ(base.CachedCount(), 1);

    ASSERT_EQ(seq1->Get(0), 1);
    ASSERT_EQ(seq1->Get(OrdinalIndexParser::Parse("w")), 99);

    ASSERT_EQ(seq2->Get(0), 100);
    ASSERT_EQ(seq2->Get(1), 1);
    ASSERT_EQ(seq2->Get(OrdinalIndexParser::Parse("w")), 99);

    delete seq1;
    delete seq2;
}

TEST_CASE(OrdinalArithmeticRemove) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> base(&gen);
    
    LazySequence<int>* seq1 = base.AppendLazy(99);
    LazySequence<int>* seq2 = seq1->PrependLazy(100);
    
    LazySequence<int>* seq3 = seq2->RemoveLazy(OrdinalIndexParser::Parse("0"));
    
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w")), 99);
    ASSERT_EQ(seq3->Get(0), 1);

    delete seq1;
    delete seq2;
    delete seq3;
}

TEST_CASE(InsertSequenceSplitting) {
    RuleGenerator<int> sqGen(TestSquare);
    LazySequence<int> base(&sqGen);

    RuleGenerator<int> cubeGen(TestCube);
    LazySequence<int>* seq = base.InsertSequence(OrdinalIndexParser::Parse("3"), &cubeGen);

    ASSERT_EQ(seq->Get(0), 1);
    ASSERT_EQ(seq->Get(2), 9);

    ASSERT_EQ(seq->Get(3), 1); 
    ASSERT_EQ(seq->Get(4), 8); 
    ASSERT_EQ(seq->Get(10), 512);

    ASSERT_EQ(seq->Get(OrdinalIndexParser::Parse("w")), 16);
    ASSERT_EQ(seq->Get(OrdinalIndexParser::Parse("w+1")), 25);

    delete seq;
}

TEST_CASE(ConcatenationShifting) {
    RuleGenerator<int> sqGen(TestSquare);
    LazySequence<int> seq1(&sqGen);
    
    LazySequence<int>* seq1_mod = seq1.AppendLazy(999);

    RuleGenerator<int> linGen(TestLinear);
    LazySequence<int> seq2(&linGen);

    LazySequence<int>* seq3 = seq1_mod->ConcatLazy(seq2);

    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w")), 999);
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w+1")), 0);
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w+5")), 40);

    delete seq1_mod;
    delete seq3;
}

TEST_CASE(DeepCopyAndMemorySafety) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> original(&gen);
    
    LazySequence<int>* seq1 = original.AppendLazy(42);
    
    LazySequence<int> copy1(*seq1);
    
    LazySequence<int> copy2;
    copy2 = *seq1;

    LazySequence<int>* copy1_mut = copy1.PrependLazy(100);

    ASSERT_EQ(seq1->Get(0), 1);
    ASSERT_EQ(copy2.Get(0), 1);
    ASSERT_EQ(seq1->Get(OrdinalIndexParser::Parse("w")), 42);
    ASSERT_EQ(copy2.Get(OrdinalIndexParser::Parse("w")), 42);

    ASSERT_EQ(copy1_mut->Get(0), 100);
    ASSERT_EQ(copy1_mut->Get(1), 1);

    delete seq1;
    delete copy1_mut;
}

TEST_CASE(SubsequenceAndEnumerator) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> seq(&gen);
    
    Sequence<int>* sub = seq.GetSubsequence(2, 5);
    ASSERT_EQ(sub->GetLength(), 4);
    ASSERT_EQ(sub->Get(0), 9);  
    ASSERT_EQ(sub->Get(1), 16);
    ASSERT_EQ(sub->Get(2), 25);
    ASSERT_EQ(sub->Get(3), 36);
    delete sub;

    IEnumerator<int>* it = seq.GetEnumerator();
    it->HasNext();
    ASSERT_EQ(it->GetCurrent(), 1);
    it->Reset();
    it->HasNext();
    ASSERT_EQ(it->GetCurrent(), 1);
    delete it;
}

TEST_CASE(ErrorHandling) {
    RuleGenerator<int> gen(TestSquare);
    LazySequence<int> seq(&gen);

    ASSERT_THROW(seq.Get(-1), IndexOutOfRangeException);
    ASSERT_THROW(seq.GetLast(), BaseException);
    ASSERT_THROW(seq.Map(TestSquare), BaseException);
    ASSERT_THROW(seq.Where([](const int& x){ return x > 0; }), BaseException);
    ASSERT_THROW(seq.Reduce([](const int& a, const int& b){ return a+b; }, 0, new int()), BaseException);
}

void RunLazySequenceTests() {
    std::cout << "========================================\n";
    std::cout << "      LazySequence Unit Tests\n";
    std::cout << "========================================\n\n";

    RUN_TEST(BasicGenerationAndCaching);
    RUN_TEST(ImmutablePrependAndAppend);
    RUN_TEST(OrdinalArithmeticRemove);
    RUN_TEST(InsertSequenceSplitting);
    RUN_TEST(ConcatenationShifting);
    RUN_TEST(DeepCopyAndMemorySafety);
    RUN_TEST(SubsequenceAndEnumerator);
    RUN_TEST(ErrorHandling);

    std::cout << "\n========================================\n";
    std::cout << "Results: \033[32m" << tests_passed << " passed\033[0m, \033[31m" << tests_failed << " failed\033[0m\n";
    std::cout << "========================================\n";

}

#endif
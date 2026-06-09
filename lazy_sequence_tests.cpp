#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include "rule_generator.h"
#include "lazy_sequence.h"
#include "ordinal_index_parser.h"
#include "base/exceptions.h"

// ==========================================
// Легковесный тестовый фреймворк
// ==========================================

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
        /* Успех: поймали ожидаемое исключение */ \
    } catch (...) { \
        throw std::runtime_error("Wrong exception type thrown for " #expr); \
    }

// ==========================================
// Вспомогательные функции-генераторы
// ==========================================

int Square(const int &i) { return (i + 1) * (i + 1); }
int Cube(const int &i)   { return (i + 1) * (i + 1) * (i + 1); }
int Linear(const int &i) { return i * 10; }

// ==========================================
// ТЕСТОВЫЕ СЛУЧАИ
// ==========================================

TEST_CASE(BasicGenerationAndCaching) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> seq(&gen);

    ASSERT_EQ(seq.Get(0), 1);
    ASSERT_EQ(seq.Get(4), 25);
    ASSERT_EQ(seq.Get(9), 100);
    
    // Проверка кэширования (мемоизации)
    ASSERT_EQ(seq.CachedCount(), 3);
    ASSERT_EQ(seq.IsCached(OrdinalIndexParser::Parse("4")), true);
}

TEST_CASE(ImmutablePrependAndAppend) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> base(&gen);

    // Создаем новые версии, оригинал не должен меняться
    LazySequence<int>* seq1 = base.AppendLazy(99);
    LazySequence<int>* seq2 = seq1->PrependLazy(100);

    // Проверяем оригинал
    ASSERT_EQ(base.Get(0), 1);
    ASSERT_EQ(base.CachedCount(), 1);

    // Проверяем seq1 (добавлено 99 в конец, т.е. на w)
    ASSERT_EQ(seq1->Get(0), 1);
    ASSERT_EQ(seq1->Get(OrdinalIndexParser::Parse("w")), 99);

    // Проверяем seq2 (добавлено 100 в начало)
    ASSERT_EQ(seq2->Get(0), 100);
    ASSERT_EQ(seq2->Get(1), 1); // Квадрат 1 сдвинулся на индекс 1
    ASSERT_EQ(seq2->Get(OrdinalIndexParser::Parse("w")), 99); // w НЕ сдвинулся! (1 + w = w)

    delete seq1;
    delete seq2;
}

TEST_CASE(OrdinalArithmeticRemove) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> base(&gen);
    
    LazySequence<int>* seq1 = base.AppendLazy(99); // 99 на w
    LazySequence<int>* seq2 = seq1->PrependLazy(100); // 100 на 0, 99 остается на w
    
    // Удаляем элемент из конечной зоны
    LazySequence<int>* seq3 = seq2->RemoveLazy(OrdinalIndexParser::Parse("0"));
    
    // Элемент на w не должен был сдвинуться
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w")), 99);
    ASSERT_EQ(seq3->Get(0), 1); // Бывший индекс 1 стал 0

    delete seq1;
    delete seq2;
    delete seq3;
}

TEST_CASE(InsertSequenceSplitting) {
    RuleGenerator<int> sqGen(Square);
    LazySequence<int> base(&sqGen);

    // Вставляем кубы, начиная с индекса 3
    RuleGenerator<int> cubeGen(Cube);
    LazySequence<int>* seq = base.InsertSequence(OrdinalIndexParser::Parse("3"), &cubeGen);

    // Левая часть квадратов: [0, 3)
    ASSERT_EQ(seq->Get(0), 1);
    ASSERT_EQ(seq->Get(2), 9);

    // Вставленные кубы: [3, w)
    ASSERT_EQ(seq->Get(3), 1); 
    ASSERT_EQ(seq->Get(4), 8); 
    ASSERT_EQ(seq->Get(10), 512);

    // Правая часть квадратов уехала на [w, 2w)
    // Индекс w соответствует бывшему локальному индексу 3 (т.е. 4^2 = 16)
    ASSERT_EQ(seq->Get(OrdinalIndexParser::Parse("w")), 16);
    ASSERT_EQ(seq->Get(OrdinalIndexParser::Parse("w+1")), 25);

    delete seq;
}

TEST_CASE(ConcatenationShifting) {
    RuleGenerator<int> sqGen(Square);
    LazySequence<int> seq1(&sqGen);
    
    // Добавляем элемент в конец seq1 (на индекс w)
    LazySequence<int>* seq1_mod = seq1.AppendLazy(999);

    RuleGenerator<int> linGen(Linear);
    LazySequence<int> seq2(&linGen);

    LazySequence<int>* seq3 = seq1_mod->ConcatLazy(seq2);

    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w")), 999);
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w+1")), 0);
    ASSERT_EQ(seq3->Get(OrdinalIndexParser::Parse("w+5")), 40);

    delete seq1_mod;
    delete seq3;
}

TEST_CASE(DeepCopyAndMemorySafety) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> original(&gen);
    
    LazySequence<int>* seq1 = original.AppendLazy(42);
    
    // 1. Конструктор копирования
    LazySequence<int> copy1(*seq1);
    
    // 2. Оператор присваивания
    LazySequence<int> copy2;
    copy2 = *seq1;

    // Мутируем копию 1
    LazySequence<int>* copy1_mut = copy1.PrependLazy(100);

    // Проверяем, что seq1 и copy2 НЕ изменились
    ASSERT_EQ(seq1->Get(0), 1);
    ASSERT_EQ(copy2.Get(0), 1);
    ASSERT_EQ(seq1->Get(OrdinalIndexParser::Parse("w")), 42);
    ASSERT_EQ(copy2.Get(OrdinalIndexParser::Parse("w")), 42);

    // Проверяем, что copy1_mut изменился
    ASSERT_EQ(copy1_mut->Get(0), 100);
    ASSERT_EQ(copy1_mut->Get(1), 1);

    delete seq1;
    delete copy1_mut;
    // Деструкторы copy1 и copy2 вызовутся автоматически. 
    // Если бы Clone() не работал, здесь был бы Double Free или Segfault.
}

TEST_CASE(SubsequenceAndEnumerator) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> seq(&gen);
    
    Sequence<int>* sub = seq.GetSubsequence(2, 5);
    ASSERT_EQ(sub->GetLength(), 4);
    ASSERT_EQ(sub->Get(0), 9);  // Square(2)
    ASSERT_EQ(sub->Get(1), 16); // Square(3)
    ASSERT_EQ(sub->Get(2), 25); // Square(4)
    ASSERT_EQ(sub->Get(3), 36); // Square(5)
    delete sub;

    // Тест Enumerator
    IEnumerator<int>* it = seq.GetEnumerator();
    it->HasNext();
    ASSERT_EQ(it->GetCurrent(), 1); // После первого HasNext() currentIndex=0
    it->Reset();
    it->HasNext();
    ASSERT_EQ(it->GetCurrent(), 1); // Снова первый элемент после сброса
    delete it;
}

TEST_CASE(ErrorHandling) {
    RuleGenerator<int> gen(Square);
    LazySequence<int> seq(&gen);

    ASSERT_THROW(seq.Get(-1), IndexOutOfRangeException);
    ASSERT_THROW(seq.GetLast(), BaseException);
    ASSERT_THROW(seq.Map(Square), BaseException);
    ASSERT_THROW(seq.Where([](const int& x){ return x > 0; }), BaseException);
    ASSERT_THROW(seq.Reduce([](const int& a, const int& b){ return a+b; }, 0, new int()), BaseException);
}

// ==========================================
// Главная функция
// ==========================================

int main() {
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

    return tests_failed > 0 ? 1 : 0;
}
#ifndef _MARKOV_UI_
#define _MARKOV_UI_

#include <iostream>
#include <string>
#include <cstring>
#include "markov_machine.h"

std::string SequenceToString(MutableArraySequence<char>* seq) {
    if (!seq) return "";
    std::string result;
    int len = seq->GetLength();
    for (int i = 0; i < len; i++) {
        result += seq->Get(i);
    }
    return result;
}

void PrintRules(MarkovMachine& mm) {
    std::cout << "\n=== Текущие правила ===" << std::endl;
    for (int i = 0; ; i++) {
        try {
            const char* pat = mm.GetRulePattern(i);
            const char* repl = mm.GetRuleReplacement(i);
            if (!pat) break;
            std::cout << "  [" << i << "] ";
            std::cout << "\"" << pat << "\" -> \"" << repl << "\"";
            std::cout << std::endl;
        } catch (...) {
            break;
        }
    }
    std::cout << "=======================\n" << std::endl;
}

void PrintState(MarkovMachine& mm) {
    std::cout << "\n--- Состояние машины ---" << std::endl;
    MutableArraySequence<char>* input = mm.GetInput();
    if (input) {
        std::cout << "  Вход: \"" << SequenceToString(input) << "\"" << std::endl;
    } else {
        std::cout << "  Вход: (не задан)" << std::endl;
    }
    std::cout << "  Шагов выполнено: " << mm.GetSteps() << std::endl;
    std::cout << "  Остановлена: " << (mm.IsHalted() ? "да" : "нет") << std::endl;
    std::cout << "------------------------\n" << std::endl;
}

void PrintMarkovHelp() {
    std::cout << "\n=== Машина Маркова — Интерактивный интерфейс ===" << std::endl;
    std::cout << "Команды:" << std::endl;
    std::cout << "  input <строка>         — установить входную строку" << std::endl;
    std::cout << "  add <шаблон> <замена> [final] — добавить правило" << std::endl;
    std::cout << "                           (final — опционально, делает правило завершающим)" << std::endl;
    std::cout << "  step                   — выполнить один шаг" << std::endl;
    std::cout << "  run                    — выполнить все шаги до остановки" << std::endl;
    std::cout << "  state                  — показать текущее состояние" << std::endl;
    std::cout << "  rules                  — показать список правил" << std::endl;
    std::cout << "  reset                  — сбросить машину (очистить вход и шаги)" << std::endl;
    std::cout << "  example1               — загрузить пример: удаление нулей" << std::endl;
    std::cout << "  example2               — загрузить пример: сложение единиц" << std::endl;
    std::cout << "  help                   — показать эту справку" << std::endl;
    std::cout << "  exit                   — выход" << std::endl;
    std::cout << "====================================================\n" << std::endl;
}

void LoadExample1(MarkovMachine& mm) {
    mm.SetInput("a0b00c0");
    mm.AddRule("0", "", false);
    std::cout << "Загружен пример 1: удаление символов '0'" << std::endl;
    std::cout << "Вход: \"a0b00c0\"" << std::endl;
    std::cout << "Правило: \"0\" -> \"\"" << std::endl;
}

void LoadExample2(MarkovMachine& mm) {
    mm.SetInput("111+11");
    mm.AddRule("1+1", "11", false);
    mm.AddRule("1+1", "11", false);
    mm.AddRule("+", "", true);     
    std::cout << "Загружен пример 2: сложение unary чисел" << std::endl;
    std::cout << "Вход: \"111+11\"" << std::endl;
    std::cout << "Правила:" << std::endl;
    std::cout << "  \"1+1\" -> \"11\"" << std::endl;
    std::cout << "  \"+\" -> \"\" (final)" << std::endl;
}

void RunMarkovUI() {
    MarkovMachine mm(10000);
    PrintMarkovHelp();

    while (true) {
        std::cout << "[markov]> ";
        std::string command;
        if (!(std::cin >> command)) break;

        if (command == "exit" || command == "quit") {
            std::cout << "Выход из программы." << std::endl;
            break;
        }
        else if (command == "help") {
            PrintMarkovHelp();
        }
        else if (command == "state") {
            PrintState(mm);
        }
        else if (command == "rules") {
            PrintRules(mm);
        }
        else if (command == "input") {
            std::string str;
            std::cin >> str;
            mm.SetInput(str.c_str());
            std::cout << "Входная строка установлена: \"" << str << "\"" << std::endl;
        }
        else if (command == "add") {
            std::string pattern, replacement;
            std::string finalFlag;
            std::cin >> pattern >> replacement;
            bool isFinal = false;
            if (std::cin >> finalFlag) {
                if (finalFlag == "final" || finalFlag == "FINAL" || finalFlag == "1") {
                    isFinal = true;
                }
            }
            mm.AddRule(pattern.c_str(), replacement.c_str(), isFinal);
            std::cout << "Добавлено правило: \"" << pattern << "\" -> \"" << replacement << "\"";
            if (isFinal) std::cout << " (final)";
            std::cout << std::endl;
        }
        else if (command == "step") {
            bool applied = mm.Step();
            if (applied) {
                MutableArraySequence<char>* input = mm.GetInput();
                std::cout << "Шаг " << mm.GetSteps() << ": \"" << SequenceToString(input) << "\"" << std::endl;
                if (mm.IsHalted()) {
                    std::cout << "Машина остановлена (финальное правило)." << std::endl;
                }
            } else {
                std::cout << "Ни одно правило не применимо. Машина остановлена." << std::endl;
            }
        }
        else if (command == "run") {
            std::cout << "Выполнение..." << std::endl;
            mm.Execute();
            MutableArraySequence<char>* input = mm.GetInput();
            std::cout << "Результат: \"" << SequenceToString(input) << "\"" << std::endl;
            std::cout << "Выполнено шагов: " << mm.GetSteps() << std::endl;
            std::cout << "Остановлена: " << (mm.IsHalted() ? "да" : "нет") << std::endl;
        }
        else if (command == "reset") {
            mm = MarkovMachine(10000);
            std::cout << "Машина сброшена." << std::endl;
        }
        else if (command == "example1") {
            LoadExample1(mm);
            std::cout << "Загружен пример 1. Выполните 'run' для результата." << std::endl;
        }
        else if (command == "example2") {
            LoadExample2(mm);
            std::cout << "Загружен пример 2. Выполните 'run' для результата." << std::endl;
        }
        else {
            std::cout << "Неизвестная команда. Введите 'help' для списка команд." << std::endl;
        }
    }

    std::cout << "Работа завершена." << std::endl;
}

#endif
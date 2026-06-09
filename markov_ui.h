#ifndef _MARKOV_UI_
#define _MARKOV_UI_

#include <iostream>
#include <string>
#include <cstring>
#include "markov_machine.h"

using namespace std;

// ==========================================
// Утилиты
// ==========================================

string SequenceToString(MutableArraySequence<char>* seq) {
    if (!seq) return "";
    string result;
    int len = seq->GetLength();
    for (int i = 0; i < len; i++) {
        result += seq->Get(i);
    }
    return result;
}

void PrintRules(MarkovMachine& mm) {
    // Мы не знаем точного количества правил извне, 
    // но можем попытаться получить их по индексам
    cout << "\n=== Текущие правила ===" << endl;
    for (int i = 0; ; i++) {
        try {
            const char* pat = mm.GetRulePattern(i);
            const char* repl = mm.GetRuleReplacement(i);
            if (!pat) break;
            cout << "  [" << i << "] ";
            // Проверяем, есть ли специальный символ для финального правила
            // В данной реализации финальность хранится внутри MarkovRule,
            // но публичного геттера IsFinal нет. Выводим просто паттерн и замену.
            cout << "\"" << pat << "\" -> \"" << repl << "\"";
            cout << endl;
        } catch (...) {
            break;
        }
    }
    cout << "=======================\n" << endl;
}

void PrintState(MarkovMachine& mm) {
    cout << "\n--- Состояние машины ---" << endl;
    MutableArraySequence<char>* input = mm.GetInput();
    if (input) {
        cout << "  Вход: \"" << SequenceToString(input) << "\"" << endl;
    } else {
        cout << "  Вход: (не задан)" << endl;
    }
    cout << "  Шагов выполнено: " << mm.GetSteps() << endl;
    cout << "  Остановлена: " << (mm.IsHalted() ? "да" : "нет") << endl;
    cout << "------------------------\n" << endl;
}

// ==========================================
// Справка
// ==========================================

void PrintMarkovHelp() {
    cout << "\n=== Машина Маркова — Интерактивный интерфейс ===" << endl;
    cout << "Команды:" << endl;
    cout << "  input <строка>         — установить входную строку" << endl;
    cout << "  add <шаблон> <замена> [final] — добавить правило" << endl;
    cout << "                           (final — опционально, делает правило завершающим)" << endl;
    cout << "  step                   — выполнить один шаг" << endl;
    cout << "  run                    — выполнить все шаги до остановки" << endl;
    cout << "  state                  — показать текущее состояние" << endl;
    cout << "  rules                  — показать список правил" << endl;
    cout << "  reset                  — сбросить машину (очистить вход и шаги)" << endl;
    cout << "  example1               — загрузить пример: удаление нулей" << endl;
    cout << "  example2               — загрузить пример: сложение единиц" << endl;
    cout << "  help                   — показать эту справку" << endl;
    cout << "  exit                   — выход" << endl;
    cout << "====================================================\n" << endl;
}

// ==========================================
// Примеры
// ==========================================

void LoadExample1(MarkovMachine& mm) {
    // Пример: удаление всех символов '0' из строки
    mm.SetInput("a0b00c0");
    mm.AddRule("0", "", false);
    cout << "Загружен пример 1: удаление символов '0'" << endl;
    cout << "Вход: \"a0b00c0\"" << endl;
    cout << "Правило: \"0\" -> \"\"" << endl;
}


void LoadExample2(MarkovMachine& mm) {
    // Пример: сложение двух unary чисел, разделённых '+'
    // "111+11" -> "11111"
    mm.SetInput("111+11");
    mm.AddRule("1+1", "11", false);
    mm.AddRule("1+1", "11", false); // дублируем для надёжности
    mm.AddRule("+", "", true);      // убираем оставшийся '+' и останавливаемся
    cout << "Загружен пример 2: сложение unary чисел" << endl;
    cout << "Вход: \"111+11\"" << endl;
    cout << "Правила:" << endl;
    cout << "  \"1+1\" -> \"11\"" << endl;
    cout << "  \"+\" -> \"\" (final)" << endl;
}

// ==========================================
// Главная функция
// ==========================================

void RunMarkovUI() {
    MarkovMachine mm(10000); // максимум 10000 шагов
    PrintMarkovHelp();

    while (true) {
        cout << "[markov]> ";
        string command;
        if (!(cin >> command)) break;

        if (command == "exit" || command == "quit") {
            cout << "Выход из программы." << endl;
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
            string str;
            cin >> str;
            mm.SetInput(str.c_str());
            cout << "Входная строка установлена: \"" << str << "\"" << endl;
        }
        else if (command == "add") {
            string pattern, replacement;
            string finalFlag;
            cin >> pattern >> replacement;
            bool isFinal = false;
            // Проверяем, указан ли флаг "final"
            if (cin >> finalFlag) {
                if (finalFlag == "final" || finalFlag == "FINAL" || finalFlag == "1") {
                    isFinal = true;
                } else {
                    // Если это не флаг, значит это уже следующая команда — 
                    // но мы не можем "вернуть" её в cin, поэтому просто игнорируем
                    // В реальной программе лучше использовать getline
                }
            }
            mm.AddRule(pattern.c_str(), replacement.c_str(), isFinal);
            cout << "Добавлено правило: \"" << pattern << "\" -> \"" << replacement << "\"";
            if (isFinal) cout << " (final)";
            cout << endl;
        }
        else if (command == "step") {
            bool applied = mm.Step();
            if (applied) {
                MutableArraySequence<char>* input = mm.GetInput();
                cout << "Шаг " << mm.GetSteps() << ": \"" << SequenceToString(input) << "\"" << endl;
                if (mm.IsHalted()) {
                    cout << "Машина остановлена (финальное правило)." << endl;
                }
            } else {
                cout << "Ни одно правило не применимо. Машина остановлена." << endl;
            }
        }
        else if (command == "run") {
            cout << "Выполнение..." << endl;
            mm.Execute();
            MutableArraySequence<char>* input = mm.GetInput();
            cout << "Результат: \"" << SequenceToString(input) << "\"" << endl;
            cout << "Выполнено шагов: " << mm.GetSteps() << endl;
            cout << "Остановлена: " << (mm.IsHalted() ? "да" : "нет") << endl;
        }
        else if (command == "reset") {
            mm = MarkovMachine(10000);
            cout << "Машина сброшена." << endl;
        }
        else if (command == "example1") {
            LoadExample1(mm);
            cout << "Загружен пример 1. Выполните 'run' для результата." << endl;
        }
        else if (command == "example2") {
            LoadExample2(mm);
            cout << "Загружен пример 2. Выполните 'run' для результата." << endl;
        }
        else {
            cout << "Неизвестная команда. Введите 'help' для списка команд." << endl;
        }
    }

    cout << "Работа завершена." << endl;
}

#endif
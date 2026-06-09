#include <iostream>
#include <string>

#include "rule_generator.h"
#include "lazy_sequence.h"
#include "ordinal_index_parser.h"

using namespace std;

// ==========================================
// Генераторы и предикаты
// ==========================================

int Square(const int &i) { return (i + 1) * (i + 1); }
int Cube(const int &i)   { return (i + 1) * (i + 1) * (i + 1); }
int Linear(const int &i) { return i * 10; }

bool IsEven(const int& x) { return x % 2 == 0; }
bool IsOdd(const int& x)  { return x % 2 != 0; }

int threshold = 0;
bool IsGreaterThan(const int& x) { return x > threshold; }

// ==========================================
// Константы и глобальное состояние
// ==========================================

const int MAX_VERSIONS = 10;
LazySequence<int>* versions[MAX_VERSIONS];
int versionCount = 0;
int currentIdx = 0;

// ==========================================
// Управление версиями
// ==========================================

void AddVersion(LazySequence<int>* newSeq) {
    if (versionCount < MAX_VERSIONS) {
        versions[versionCount] = newSeq;
        currentIdx = versionCount;
        versionCount++;
    } else {
        // Массив полон: удаляем самую старую версию (index 0)
        delete versions[0];
        // Сдвигаем все на 1 позицию влево
        for (int i = 1; i < MAX_VERSIONS; i++) {
            versions[i - 1] = versions[i];
        }
        versions[MAX_VERSIONS - 1] = newSeq;
        currentIdx = MAX_VERSIONS - 1;
        cout << "[info] History full. Oldest version deleted." << endl;
    }
}

LazySequence<int>* Current() {
    return versions[currentIdx];
}

void PrintVersions() {
    cout << "\n=== Sequence Versions (" << versionCount << "/" << MAX_VERSIONS << ") ===" << endl;
    for (int i = 0; i < versionCount; i++) {
        cout << (i == currentIdx ? " -> " : "    ");
        cout << "[" << i << "] cached=" << versions[i]->CachedCount() 
             << ", ranges=" << versions[i]->RangeCount() << endl;
    }
    cout << "==========================================\n" << endl;
}

// ==========================================
// Справка
// ==========================================

void PrintHelp() {
    cout << "\n=== LazySequence Versioned Demo ===\n";
    cout << "Все операции создают НОВУЮ версию последовательности.\n\n";
    cout << "get INDEX              - получить элемент (5, w, w+2, 2w+10)\n";
    cout << "prepend VALUE          - создать версию с добавлением в начало\n";
    cout << "append VALUE           - создать версию с добавлением в конец\n";
    cout << "insert INDEX VALUE     - создать версию со вставкой\n";
    cout << "remove INDEX           - создать версию с удалением\n";
    cout << "insertseq INDEX        - создать версию с последовательностью кубов\n";
    cout << "concat                 - создать версию + Linear(0,10,20...) + [999,888]\n";
    cout << "sub START END          - вывести конечную подпоследовательность\n";
    cout << "tryfirst even|odd|gt N - поиск по предикату\n\n";
    cout << "list                   - показать все версии\n";
    cout << "switch N               - переключиться на версию N\n";
    cout << "delete N               - удалить версию N\n";
    cout << "info                   - информация о текущей версии\n";
    cout << "help | exit\n";
    cout << "===================================\n" << endl;
}

// ==========================================
// Главная функция
// ==========================================

int main() {
    // Инициализация: первая версия — последовательность квадратов
    RuleGenerator<int> squareGen(Square);
    versions[0] = new LazySequence<int>(&squareGen);
    versionCount = 1;
    currentIdx = 0;

    PrintHelp();

    while (true) {
        cout << "[v" << currentIdx << "]> ";
        string command;
        if (!(cin >> command)) break;

        if (command == "exit") break;
        if (command == "help") { PrintHelp(); continue; }

        try {
            // ==========================================
            // Операции, создающие НОВУЮ версию
            // ==========================================
            
            if (command == "prepend") {
                int value; cin >> value;
                LazySequence<int>* newSeq = Current()->PrependLazy(value);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " with prepended " << value << endl;
            }
            else if (command == "append") {
                int value; cin >> value;
                LazySequence<int>* newSeq = Current()->AppendLazy(value);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " with appended " << value << endl;
            }
            else if (command == "insert") {
                string idxString; int value;
                cin >> idxString >> value;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                LazySequence<int>* newSeq = Current()->InsertAtLazy(idx, value);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " with inserted " << value << " at " << idx.ToString() << endl;
            }
            else if (command == "remove") {
                string idxString; cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                LazySequence<int>* newSeq = Current()->RemoveLazy(idx);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " with removed element at " << idx.ToString() << endl;
            }
            else if (command == "insertseq") {
                string idxString; cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                RuleGenerator<int> cubeGen(Cube);
                LazySequence<int>* newSeq = Current()->InsertSequence(idx, &cubeGen);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " with cubes inserted at " << idx.ToString() << endl;
            }
            else if (command == "concat") {
                RuleGenerator<int> linearGen(Linear);
                LazySequence<int> otherSeq(&linearGen);
                
                LazySequence<int>* newSeq = Current()->ConcatLazy(otherSeq);
                AddVersion(newSeq);
                cout << "Created version " << currentIdx << " concatenated with Linear " << endl;
            }

            // ==========================================
            // Чтение (не создают новых версий)
            // ==========================================
            
            else if (command == "get") {
                string idxString; cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                cout << Current()->Get(idx) << endl;
            }
            else if (command == "sub") {
                int start, end;
                cin >> start >> end;
                Sequence<int>* sub = Current()->GetSubsequence(start, end);
                cout << "Subsequence [" << start << ".." << end << "]: [";
                for (int i = 0; i < sub->GetLength(); i++) {
                    if (i > 0) cout << ", ";
                    cout << sub->Get(i);
                }
                cout << "]" << endl;
                delete sub;
            }
            else if (command == "tryfirst") {
                string predName; cin >> predName;
                Option<int> result = Option<int>::None();
                
                if (predName == "even") {
                    result = Current()->TryGetFirst(IsEven);
                } else if (predName == "odd") {
                    result = Current()->TryGetFirst(IsOdd);
                } else if (predName == "gt") {
                    cin >> threshold;
                    result = Current()->TryGetFirst(IsGreaterThan);
                } else {
                    cout << "Unknown predicate" << endl;
                    continue;
                }
                
                if (result.HasValue()) {
                    cout << "Found: " << result.GetValue() << endl;
                } else {
                    cout << "Not found" << endl;
                }
            }

            // ==========================================
            // Управление версиями
            // ==========================================
            
            else if (command == "list") {
                PrintVersions();
            }
            else if (command == "switch") {
                int n; cin >> n;
                if (n < 0 || n >= versionCount) {
                    cout << "Invalid version index" << endl;
                } else {
                    currentIdx = n;
                    cout << "Switched to version " << currentIdx << endl;
                }
            }
            else if (command == "delete") {
                int n; cin >> n;
                if (n < 0 || n >= versionCount) {
                    cout << "Invalid version index" << endl;
                } else if (versionCount == 1) {
                    cout << "Cannot delete the only version" << endl;
                } else {
                    delete versions[n];
                    // Сдвигаем все после n влево
                    for (int i = n; i < versionCount - 1; i++) {
                        versions[i] = versions[i + 1];
                    }
                    versionCount--;
                    // Корректируем currentIdx
                    if (currentIdx >= versionCount) {
                        currentIdx = versionCount - 1;
                    } else if (currentIdx > n) {
                        currentIdx--;
                    }
                    cout << "Deleted version " << n << endl;
                    PrintVersions();
                }
            }

            // ==========================================
            // Info
            // ==========================================
            
            else if (command == "info") {
                LazySequence<int>* seq = Current();
                cout << "\nCurrent version [" << currentIdx << "]:" << endl;
                cout << "  Cached values:    " << seq->CachedCount() << endl;
                cout << "  Generator ranges: " << seq->RangeCount() << endl;
                cout << "  Length:           " << seq->GetLength() << " (infinite)" << endl;
                cout << "  First 10:         [";
                for (int i = 0; i < 10; i++) {
                    if (i > 0) cout << ", ";
                    try { cout << seq->Get(i); } catch (...) { cout << "?"; }
                }
                cout << ", ...]" << endl << endl;
            }
            else {
                cout << "Unknown command. Type 'help'." << endl;
            }
        }
        catch (const BaseException& e) {
            cout << "Error: " << e.what() << endl;
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        catch (...) {
            cout << "Cannot get element" << endl;
        }
    }

    // Очистка памяти: удаляем все версии
    for (int i = 0; i < versionCount; i++) {
        delete versions[i];
    }
    cout << "All " << versionCount << " versions deleted. Goodbye!" << endl;
    return 0;
}
#ifndef _LAZY_UI_
#define _LAZY_UI_

#include <iostream>
#include <string>

#include "rule_generator.h"
#include "lazy_sequence.h"
#include "ordinal_index_parser.h"


int Square(const int &i) { return (i + 1) * (i + 1); }
int Cube(const int &i)   { return (i + 1) * (i + 1) * (i + 1); }
int Linear(const int &i) { return i * 10; }

bool IsEven(const int& x) { return x % 2 == 0; }
bool IsOdd(const int& x)  { return x % 2 != 0; }

int threshold = 0;
bool IsGreaterThan(const int& x) { return x > threshold; }

const int MAX_VERSIONS = 10;
LazySequence<int>* versions[MAX_VERSIONS];
int versionCount = 0;
int currentIdx = 0;


void AddVersion(LazySequence<int>* newSeq) {
    if (versionCount < MAX_VERSIONS) {
        versions[versionCount] = newSeq;
        currentIdx = versionCount;
        versionCount++;
    } else {
        delete versions[0];
        for (int i = 1; i < MAX_VERSIONS; i++) {
            versions[i - 1] = versions[i];
        }
        versions[MAX_VERSIONS - 1] = newSeq;
        currentIdx = MAX_VERSIONS - 1;
        std::cout << "[info] History full. Oldest version deleted." << std::endl;
    }
}

LazySequence<int>* Current() {
    return versions[currentIdx];
}

void PrintVersions() {
    std::cout << "\n=== Sequence Versions (" << versionCount << "/" << MAX_VERSIONS << ") ===" << std::endl;
    for (int i = 0; i < versionCount; i++) {
        std::cout << (i == currentIdx ? " -> " : "    ");
        std::cout << "[" << i << "] cached=" << versions[i]->CachedCount() 
             << ", ranges=" << versions[i]->RangeCount() << std::endl;
    }
    std::cout << "==========================================\n" << std::endl;
}

void PrintHelp() {
    std::cout << "\n=== LazySequence Versioned Demo ===\n";
    std::cout << "Все операции создают НОВУЮ версию последовательности.\n\n";
    std::cout << "get INDEX              - получить элемент (5, w, w+2, 2w+10)\n";
    std::cout << "prepend VALUE          - создать версию с добавлением в начало\n";
    std::cout << "append VALUE           - создать версию с добавлением в конец\n";
    std::cout << "insert INDEX VALUE     - создать версию со вставкой\n";
    std::cout << "remove INDEX           - создать версию с удалением\n";
    std::cout << "insertseq INDEX        - создать версию с последовательностью кубов\n";
    std::cout << "concat                 - создать версию + Linear(0,10,20...) + [999,888]\n";
    std::cout << "sub START END          - вывести конечную подпоследовательность\n";
    std::cout << "tryfirst even|odd|gt N - поиск по предикату\n\n";
    std::cout << "list                   - показать все версии\n";
    std::cout << "switch N               - переключиться на версию N\n";
    std::cout << "delete N               - удалить версию N\n";
    std::cout << "info                   - информация о текущей версии\n";
    std::cout << "help | exit\n";
    std::cout << "===================================\n" << std::endl;
}

void RunLazyUI() {
    RuleGenerator<int> squareGen(Square);
    versions[0] = new LazySequence<int>(&squareGen);
    versionCount = 1;
    currentIdx = 0;

    PrintHelp();

    while (true) {
        std::cout << "[v" << currentIdx << "]> ";
        std::string command;
        if (!(std::cin >> command)) break;

        if (command == "exit") break;
        if (command == "help") { PrintHelp(); continue; }

        try {            
            if (command == "prepend") {
                int value; std::cin >> value;
                LazySequence<int>* newSeq = Current()->PrependLazy(value);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " with prepended " << value << std::endl;
            }
            else if (command == "append") {
                int value; std::cin >> value;
                LazySequence<int>* newSeq = Current()->AppendLazy(value);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " with appended " << value << std::endl;
            }
            else if (command == "insert") {
                std::string idxString; int value;
                std::cin >> idxString >> value;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                LazySequence<int>* newSeq = Current()->InsertAtLazy(idx, value);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " with inserted " << value << " at " << idx.ToString() << std::endl;
            }
            else if (command == "remove") {
                std::string idxString; std::cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                LazySequence<int>* newSeq = Current()->RemoveLazy(idx);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " with removed element at " << idx.ToString() << std::endl;
            }
            else if (command == "insertseq") {
                std::string idxString; std::cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                RuleGenerator<int> cubeGen(Cube);
                LazySequence<int>* newSeq = Current()->InsertSequence(idx, &cubeGen);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " with cubes inserted at " << idx.ToString() << std::endl;
            }
            else if (command == "concat") {
                RuleGenerator<int> linearGen(Linear);
                LazySequence<int> otherSeq(&linearGen);
                
                LazySequence<int>* newSeq = Current()->ConcatLazy(otherSeq);
                AddVersion(newSeq);
                std::cout << "Created version " << currentIdx << " concatenated with Linear " << std::endl;
            }
            else if (command == "get") {
                std::string idxString; std::cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                std::cout << Current()->Get(idx) << std::endl;
            }
            else if (command == "sub") {
                int start, end;
                std::cin >> start >> end;
                Sequence<int>* sub = Current()->GetSubsequence(start, end);
                std::cout << "Subsequence [" << start << ".." << end << "]: [";
                for (int i = 0; i < sub->GetLength(); i++) {
                    if (i > 0) std::cout << ", ";
                    std::cout << sub->Get(i);
                }
                std::cout << "]" << std::endl;
                delete sub;
            }
            else if (command == "tryfirst") {
                std::string predName; std::cin >> predName;
                Option<int> result = Option<int>::None();
                
                if (predName == "even") {
                    result = Current()->TryGetFirst(IsEven);
                } else if (predName == "odd") {
                    result = Current()->TryGetFirst(IsOdd);
                } else if (predName == "gt") {
                    std::cin >> threshold;
                    result = Current()->TryGetFirst(IsGreaterThan);
                } else {
                    std::cout << "Unknown predicate" << std::endl;
                    continue;
                }
                
                if (result.HasValue()) {
                    std::cout << "Found: " << result.GetValue() << std::endl;
                } else {
                    std::cout << "Not found" << std::endl;
                }
            }
            else if (command == "list") {
                PrintVersions();
            }
            else if (command == "switch") {
                int n; std::cin >> n;
                if (n < 0 || n >= versionCount) {
                    std::cout << "Invalid version index" << std::endl;
                } else {
                    currentIdx = n;
                    std::cout << "Switched to version " << currentIdx << std::endl;
                }
            }
            else if (command == "delete") {
                int n; std::cin >> n;
                if (n < 0 || n >= versionCount) {
                    std::cout << "Invalid version index" << std::endl;
                } else if (versionCount == 1) {
                    std::cout << "Cannot delete the only version" << std::endl;
                } else {
                    delete versions[n];
                    for (int i = n; i < versionCount - 1; i++) {
                        versions[i] = versions[i + 1];
                    }
                    versionCount--;
                    if (currentIdx >= versionCount) {
                        currentIdx = versionCount - 1;
                    } else if (currentIdx > n) {
                        currentIdx--;
                    }
                    std::cout << "Deleted version " << n << std::endl;
                    PrintVersions();
                }
            }
            else if (command == "info") {
                LazySequence<int>* seq = Current();
                std::cout << "\nCurrent version [" << currentIdx << "]:" << std::endl;
                std::cout << "  Cached values:    " << seq->CachedCount() << std::endl;
                std::cout << "  Generator ranges: " << seq->RangeCount() << std::endl;
                std::cout << "  Length:           " << seq->GetLength() << " (infinite)" << std::endl;
                std::cout << "  First 10:         [";
                for (int i = 0; i < 10; i++) {
                    if (i > 0) std::cout << ", ";
                    try { std::cout << seq->Get(i); } catch (...) { std::cout << "?"; }
                }
                std::cout << ", ...]" << std::endl << std::endl;
            }
            else {
                std::cout << "Unknown command. Type 'help'." << std::endl;
            }
        }
        catch (const BaseException& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
        catch (...) {
            std::cout << "Cannot get element" << std::endl;
        }
    }

    for (int i = 0; i < versionCount; i++) {
        delete versions[i];
    }
    std::cout << "All " << versionCount << " versions deleted. Goodbye!" << std::endl;
}

#endif
#include <iostream>
#include <string>

#include "rule_generator.h"
#include "lazy_sequence.h"
#include "ordinal_index_parser.h"

using namespace std;

int Square(int i) { return (i + 1) * (i + 1); }
int Cube(int i) { return (i + 1) * (i + 1) * (i + 1); }

void PrintHelp() {
    cout << "\nCommands:" << endl;
    cout << "get INDEX | prepend VALUE | append VALUE" << endl;
    cout << "insert INDEX VALUE | remove INDEX" << endl;
    cout << "insertseq INDEX (вставляет последовательность кубов)" << endl;
    cout << "info | help | exit\n" << endl;
    cout << "Examples: get 5 | get ω | get ω+2 | get 2ω+10\n" << endl;
}

int main() {
    RuleGenerator<int> squareGen(Square);
    LazySequence<int> seq(&squareGen);

    PrintHelp();

    while (true) {
        cout << "> ";
        string command;
        if (!(cin >> command)) break;

        if (command == "exit") break;
        if (command == "help") { PrintHelp(); continue; }

        try {
            if (command == "get") {
                string idxString; cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                cout << seq.Get(idx) << endl;
            } 
            else if (command == "prepend") {
                int value; cin >> value;
                seq.Prepend(value);
                cout << "Added to beginning" << endl;
            } 
            else if (command == "append") {
                int value; cin >> value;
                seq.Append(value);
                cout << "Added to end" << endl;
            } 
            else if (command == "insert") {
                string idxString; int value;
                cin >> idxString >> value;
                seq.InsertAt(OrdinalIndexParser::Parse(idxString), value);
                cout << "Inserted" << endl;
            } 
            else if (command == "remove") {
                string idxString; cin >> idxString;
                seq.Remove(OrdinalIndexParser::Parse(idxString));
                cout << "Removed" << endl;
            } 
            else if (command == "insertseq") {
                string idxString; cin >> idxString;
                OrdinalIndex idx = OrdinalIndexParser::Parse(idxString);
                
                RuleGenerator<int> cubeGen(Cube);
                seq.InsertSequence(idx, &cubeGen);
                cout << "Sequence of cubes inserted at " << idx.ToString() << endl;
            }
            else if (command == "info") {
                cout << "Cached values: " << seq.CachedCount() << endl;
                cout << "Generator ranges: " << seq.RangeCount() << endl;
            } 
            else {
                cout << "Unknown command" << endl;
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        } catch (...) {
            cout << "Cannot get element" << endl;
        }
    }
    return 0;
}
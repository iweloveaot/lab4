#include "lazy_sequence_ui.h"
#include "lazy_sequence_tests.h"
#include "markov_ui.h"
#include "markov_tests.h"

int main() {
    int choice;
    do {
        std::cout << "\n ---  LAZY SEQUENCE & MARKOV MACHINE  --- \n";
        std::cout << "1. work with Lazy Sequence \n";
        std::cout << "2. work with Markov Machine \n";
        std::cout << "3. run Lazy Sequence tests \n";
        std::cout << "4. run Markov Machine tests \n";
        std::cout << "0. Exit \n";
        std::cout << "Choice: ";

        while (!(std::cin >> choice)) {
            std::cout << "Invalid input. Please enter command 0 to 3: ";
            std::cin.clear();
            std::cin.ignore(10000, '\n');
        }
        std::cin.ignore(10000, '\n');
        
        try {
            switch (choice) {
                case 1: {
                    RunLazyUI();
                    break;
                }
                case 2: {
                    RunMarkovUI();
                    break;
                }
                case 3: {
                    RunLazySequenceTests();
                    break;
                }
                case 4: {
                    RunMarkovMachineTests();
                    break;
                }
                case 0:
                    std::cout << "Bye-bye!\n";
                    break;
                default:
                    std::cout << "Invalid choice!\n";
            }
        } catch (const std::exception& e) {
            std::cerr << "Unhandled error: " << e.what() << "\n";
        }
    } while (choice != 0);
    
    return 0;
}
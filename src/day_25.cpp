#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
#include "intcode.h"
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using namespace boost::multiprecision;
using namespace boost;


/**
 * Day 25 - Cryostasis
 *
 * A small text adventure in Intcode. This is a combination of manual and programmatic solution:
 *
 * 1. explore the adventure manually
 * 2. feed inputs and all item combinations programmatically in a 2nd step
 */

class Adventure : public Program
{
    public:
        Adventure(memory_t *mem) : Program(mem) {}

        /**
         * Asks for input from the command line once the input queue is empty
         */
        virtual cpp_int getInputValue() {
            if (this->inputValues.size() == 0) {
                string input;
                cout << ": ";
                getline(cin, input);
                this->asciiInput(input);
            }
            cpp_int val = this->inputValues.front();
            this->inputValues.pop();
            cout << static_cast<char>(val);
            return val;
        }
};

/**
 * Builds a list with all possible combinations of "items". The items here are
 * digits from 1 to (max) 9, so it finds all combinations possible
 * from 1 to 123456789
 *
 * This is then used as inventory item index later
 */
void buildAllCombinations(long nr, vector<string> &combs)
{
    long maxNr;
    string tmp;
    for (long i = 0; i < nr; i++) {
        tmp += to_string(nr);
    }
    maxNr = stol(tmp);
    for (long i = 1; i <= maxNr; ++i) {
        tmp = to_string(i);
        if (i < 10) {
            combs.push_back(tmp);
            continue;
        }
        bool found = true;
        for (unsigned int idx = 1; idx < tmp.length(); ++idx) {
            if (tmp[idx-1] >= tmp[idx] || tmp[idx] > (nr + 48)) {
                found = false;
                break;
            }
        }
        if (found == true) {
            combs.push_back(tmp);
        }
    }
}

/**
*/
void solution1(memory_t &data)
{

    Adventure program(&data);
    string last_output = "";
    string input = "";
    vector<string> inventory;

    // as initial walkthrough, we provide a list of (known) commands so far, so that I dont have to enter it manually.
    // The program uses them, and asks for manual input if the list is done.
    program.asciiInput("south");
    program.asciiInput("west");
    program.asciiInput("north"); // in crew quarters
    program.asciiInput("take fuel cell");
    inventory.push_back("fuel cell");
    program.asciiInput("south");
    program.asciiInput("east");
    program.asciiInput("north"); // back on start
    program.asciiInput("north"); // stables
    program.asciiInput("east"); // gift wrap center
    program.asciiInput("take candy cane"); // gift wrap center
    inventory.push_back("candy cane");
    program.asciiInput("south");
    program.asciiInput("take hypercube");
    inventory.push_back("hypercube");
    program.asciiInput("north");
    program.asciiInput("west"); // stables
    program.asciiInput("north"); // observatory
    program.asciiInput("take coin");
    inventory.push_back("coin");
    program.asciiInput("east");
    program.asciiInput("take tambourine");
    inventory.push_back("tambourine");
    program.asciiInput("west");
    program.asciiInput("west"); //Arcade
    program.asciiInput("take spool of cat6");
    inventory.push_back("spool of cat6");
    program.asciiInput("north"); // Nav
    program.asciiInput("take weather machine");
    inventory.push_back("weather machine");
    program.asciiInput("west");
    program.asciiInput("take mutex");
    inventory.push_back("mutex");
    program.asciiInput("west"); // security checkpoint

    // Inventory combinations: we have 8 inventory combinations so far,
    // so build up a list of all possible item combinations:
    vector<string> invCombinations;
    cout << "Please stand by, initializing item combinations ..." << endl;
    buildAllCombinations(inventory.size(), invCombinations);
    cout << "Found " << invCombinations.size() << " combinations, let's start." << endl;

    // Try any of the item combinations to check whether it takes me to the next room or not:
    for (auto comb : invCombinations) {
        // drop all inv items before the security chekpoint:
        for (auto i : inventory) {
            program.asciiInput("drop " + i);
        }
        // take up the inventory item:
        for (auto c : comb) {
            program.asciiInput("take " + inventory[c-49]);
        }

        // now, head west through the security floor:
        program.asciiInput("west");
    }

    // Yes, it did! The following combination lead to a solution:
    /**
     * - hypercube
     * - tambourine
     * - spool of cat6
     * - weather machine
     */


    // Run the adventure!
    while (program.state != P_HALT)
    {
        program.runProgram();
        if (program.output == 10) {
            last_output = "";
        } else {
            last_output += static_cast<char>( program.output );
        }
        cout << static_cast<char>(program.output);
    }
}

int main(int argc, char *args[])
{
    // bool display = argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false;
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    solution1(data);
}

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
 * Day 21 - Springdroid Adventure
 */





/**
 * The robot jumps 4 tiles
 * A - 1 tile away
 * B - 2 tiles away
 * C - 3 tiles away
 * D - 4 tiles away
 *
 * J: Jump reg
 * T: Temp reg
 *
 * AND X Y --> Y = X & Y
 * OR X Y  --> Y = X | Y
 * NOT X Y --> Y = !X
 *
 * So we have to deal with a logic puzzle.
 *
 * The robot should jump, if the 4th tile is solid (1),
 * but any one between is a hole. So this can be expressed as logic statements:
 *
 * J = (!A | !B | !C) & D
 *
 * Only problem: we have to deal with J and T as storage only.
 */
void solution1(memory_t &data)
{
    Program program(&data);
    vector<string> commands{
        "NOT A J", // !A --> J
            "NOT B T", // !B --> T
            "OR T J",  // !A | !B --> J
            "NOT C T", // !C --> T
            "OR T J",  // !A | !B | !C --> J
            "AND D J",  // J & D --> J
            "WALK"
    };

    // Fill inputs:
    for (auto c : commands)
    {
        program.asciiInput(c);
    }

    cpp_int last_output;
    while (program.state != P_HALT) {
        program.runProgram();
        last_output = program.output;
        if (last_output <= 255) {
            cout << static_cast<char>(last_output);
        }
    }
    cout << "Solution 1: " << last_output << endl;
    cout << endl;
}

/**       .
 * Solution 2:

 Add the requirement: AND (5th or 8th) must be a tile, too.
 This way the robot moves nearer to the abyss until jumping:

 So add AND (E OR H):
 */
void solution2(memory_t &data) {
    Program program(&data);
    vector<string> commands{
        // (!A | !B | !C) & D (from solution 1)
        "NOT A J", // !A --> J
            "NOT B T", // !B --> T
            "OR T J",  // !A | !B --> J
            "NOT C T", // !C --> T
            "OR T J",  // !A | !B | !C --> J
            "AND D J",  // J & D --> J

            // AND (E OR H)
            "NOT E T", // Store E value to T, by double-inverting it
            "NOT T T",
            "OR H T", // OR E with H
            "AND T J", // AND (E OR H)
            "RUN"
    };

    // Fill inputs:
    for (auto c : commands)
    {
        program.asciiInput(c);
    }

    cpp_int last_output;
    while (program.state != P_HALT) {
        program.runProgram();
        last_output = program.output;
        if (last_output <= 255) {
            cout << static_cast<char>(last_output);
        }
    }
    cout << "Solution 2: " << last_output << endl;
    cout << endl;
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
    solution2(data);
}

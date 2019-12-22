#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include "common.h"
#include "intcode.h"

using namespace std;

/**
 * Day 5 - Sunny with a Chance of Asteroids
 */


int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    memory_t data;
    readIntcodeProgramFromFile(args[1], data);

    // Solution 1: Input data: 1
    Program p(&data);
    p.inputValues.push(1);
    while (p.state != P_HALT) {
        p.runProgram();
    }
    cout << "Solution 1: Output after program ends: " << p.output << endl << endl;

    // Solution 2: Input data: 5
    Program p2 = Program(&data);
    p2.inputValues.push(5);
    while (p2.state != P_HALT) {
        p2.runProgram();
    }
    cout << "Solution 2: Output after program ends: " << p2.output << endl;
}

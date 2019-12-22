#include <iostream>
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
 * Day 9 - Sensor Boost
 *
 * The program now uses a map as memory, instead of an array: Each address is the key. This supports
 * arbitary large (uninitialized) memory.
 *
 * Besides, to support large numbers, I use the boost datatype cpp_int.
 *
 * So this code needs the boost library, using boost::multiprecision::cpp_int.
 */


void runSolution1(memory_t &data)
{
    Program program(&data);
    program.inputValues.push(cpp_int(1));
    program.runProgram();
    cpp_int output = program.output;
    cout << "Solution 1: Output: " << output << endl;
}

void runSolution2(memory_t &data)
{
    Program program(&data);
    program.inputValues.push(cpp_int(2));
    program.runProgram();
    cpp_int output = program.output;
    cout << "Solution 2: Output: " << output << endl;
}

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

    runSolution1(data);
    runSolution2(data);
}

#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 5 - Sunny with a Chance of Asteroids
 */

vector<long> inputData;
vector<long>::size_type inputDataPointer;

/**
 * extracts thae param modes from the whole opcode number.
 * j
 * Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
 * "10102": --> "101" are param modes, "02" is the opcode 2.
 * The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
 * the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
 *
 * The modes vector is filled with the param's opmode, in the CORRECT order (element 0: 1st param, element 1: 2nd param and so on)
 */
void readParamModesFromOpcode(long opcode, vector<int> &modes) {
    string str = to_string(opcode);
    string modesStr{""};
    if (str.length() > 2) {
        modesStr = str.substr(0, str.length()-2);
    }
    // cout << "  Input opcode:" << str << endl;
    for (vector<int>::size_type i = 0; i < modes.size(); i++) {
        if (i < modesStr.length()) {
            char mode = modesStr[modesStr.length()-1-i];
            switch (mode) {
                case '0': modes[i] = 0; break;
                case '1': modes[i] = 1; break;
            }
        }
    }
    // cout << "  Param modes" << modes[0] << ":"<<modes[1] <<":"<<modes[2] << endl;
}

/**
 * get value from a data position, either by pointer or by value, depending on the param op mode.
 *
 * if paramMode is 0, this means position mode: The value at iPointer is a POINTER to the actual value
 * if paramMode is 1, this means immediate mode: The value at iPointer is the actual value
 */
long getValue(vector<long> &data, vector<long>::size_type iPointer, int paramMode) {
    if (paramMode == 0) {
        // position mode: data[iPointer] is a position pointer
        long valPos = data[iPointer];
        return data[valPos];
    } else if (paramMode == 1) {
        // immediate mode: data[iPointer] is the value
        return data[iPointer];
    } else {
        cerr << "Unknown param mode: " << paramMode << endl;
        exit(1);
    }
}

/**
 * Executes a single instruction:
 * @param iPointer The actual instruction pointer
 * @param data The memory. iPointer should now point to an opcode value
 */
vector<long>::size_type executeInstruction(vector<long>::size_type iPointer, vector<long> &data)
{
    // Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
    // "10102": --> "101" are param modes, "02" is the opcode 2.
    // The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
    // the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
    long opcode = data[iPointer] % 100; // last 2 digits are the opcode
    // cout << "I:" << iPointer << ": OP:" << opcode << "(" << data[iPointer] << ")" << endl;

    vector<int> paramModes{0, 0, 0};
    readParamModesFromOpcode(data[iPointer], paramModes);

    long val1;
    long val2;
    long storePos;
    long inputValue;
    switch (opcode)
    {
    // Add instr:
    case 1:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        data[storePos] = val1 + val2;
        iPointer += 4;
        break;
    // Multiply instr:
    case 2:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        data[storePos] = val1 * val2;
        iPointer += 4;
        break;
    // Input instr.
    case 3:
        inputValue = inputData[inputDataPointer++];
        storePos = data[iPointer + 1];
        data[storePos] = inputValue;
        // cout << "    Input value taken: " << inputValue << ", store at: " << storePos << endl;
        iPointer += 2;
        break;
    // Output instr.
    case 4:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        cout << "    Output: " << val1 << endl;
        iPointer += 2;
        break;
    // Jump if true:
    case 5:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        if (val1 != 0) {
            iPointer = val2;
        } else {
            iPointer += 3;
        }
        break;
    // Jump if false:
    case 6:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        if (val1 == 0) {
            iPointer = val2;
        } else {
            iPointer += 3;
        }
        break;
    // less than
    case 7:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        if (val1 < val2) {
            data[storePos] = 1;
        } else {
            data[storePos] = 0;
        }
        iPointer += 4;
        break;
    // equals:
    case 8:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        if (val1 == val2) {
            data[storePos] = 1;
        } else {
            data[storePos] = 0;
        }
        iPointer += 4;
        break;
    case 99:
        iPointer = data.size();
        break;
    default:
        cerr << "Error: unknown opcode occured: " << opcode << endl;
        exit(1);
    }

    return iPointer;
}

/**
 * Run program until the end, instruction-by-instruction
 */
long runProgram(vector<long> &data)
{
    vector<long>::size_type instructionPointer = 0;
    while (instructionPointer < data.size())
    {
        instructionPointer = executeInstruction(instructionPointer, data);
    }
    return data[0];
}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<long> data;
    readData<long>(args[1], ',', data);

    // Solution 1: Input data: 1
    inputData = vector<long>{1};
    inputDataPointer = 0;
    vector<long> workingData(data);
    runProgram(workingData);
    cout << "Solution 1: Data at pos 0 after program end: " << workingData[0] << endl << endl;

    // Solution 2: Input data: 5
    inputData = vector<long>{5};
    inputDataPointer = 0;
    workingData = vector<long>(data);
    runProgram(workingData);
    cout << "Solution 2: Data at pos 0 after program end: " << workingData[0] << endl;
}

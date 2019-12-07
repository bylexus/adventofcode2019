#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <math.h>
#include <vector>
#include <map>
#include "common.h"

using namespace std;

/**
 * Day 7 Part 2 - Amplification Circuit
 *
 * Now the program becomes really ugly.... It was not meant to be used in that way,
 * so I hacked a lot with global variables and input data reusage...
 *
 * Don't look too closely: If you close your eyes, it looks the best :-))
 *
 * I will rewrite this crappy piece of software later. (yes, sure ...)
 */

typedef vector<long> memory_t;

// Globals, used on all programs

// I use 2 input variables: inputData[0] contains the actual initial permutation value,
// inputData[2] is the last output (0 in the beginning) from an AMP, and taken as further inputs.
vector<long> inputData{0, 0};

// Points to the actual inputData index that should be read next.
vector<long>::size_type inputDataPointer = 0;

// An array with instruction pointers, one for each running AMP.
vector<memory_t::size_type> instructionPointers;

// If an AMP outputs, it stores its output to this global variable. Yes, ugly!
long output;

/**
 * An AMP can be in one of the following states:
 * - P_RUN: still running
 * - P_OUTPUT: Program has generated an output, and is halted until continued
 * - P_HALT: The program is done, no furter run
 */
enum ProgramResult {P_RUN, P_OUTPUT, P_HALT};


/**
 * extracts thae param modes from the whole opcode number.
 * Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
 * "10102": --> "101" are param modes, "02" is the opcode 2.
 * The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
 * the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
 *
 * The modes vector is filled with the param's opmode, in the CORRECT order (element 0: 1st param, element 1: 2nd param and so on)
 */
void readParamModesFromOpcode(long opcode, vector<int> &modes)
{
    string str = to_string(opcode);
    string modesStr{""};
    if (str.length() > 2)
    {
        modesStr = str.substr(0, str.length() - 2);
    }
    // cout << "  Input opcode:" << str << endl;
    for (vector<int>::size_type i = 0; i < modes.size(); i++)
    {
        if (i < modesStr.length())
        {
            char mode = modesStr[modesStr.length() - 1 - i];
            switch (mode)
            {
            case '0':
                modes[i] = 0;
                break;
            case '1':
                modes[i] = 1;
                break;
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
long getValue(memory_t &data, memory_t::size_type iPointer, int paramMode)
{
    if (paramMode == 0)
    {
        // position mode: data[iPointer] is a position pointer
        long valPos = data[iPointer];
        return data[valPos];
    }
    else if (paramMode == 1)
    {
        // immediate mode: data[iPointer] is the value
        return data[iPointer];
    }
    else
    {
        cerr << "Unknown param mode: " << paramMode << endl;
        exit(1);
    }
}

/**
 * Executes a single instruction:
 * @param iPointer The actual instruction pointer. Is MODIFIED directly by the procedure
 * @param data The memory. iPointer should now point to an opcode value
 *
 * Returns the program Result. Output is written to the global "output" variable
 */
ProgramResult executeInstruction(memory_t::size_type &iPointer, memory_t &data)
{
    // Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
    // "10102": --> "101" are param modes, "02" is the opcode 2.
    // The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
    // the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
    long opcode = data[iPointer] % 100; // last 2 digits are the opcode
    // cout << "I:" << iPointer << ": OP:" << opcode << "(" << data[iPointer] << ")" << endl;

    if (iPointer >= data.size()) {
        return P_HALT;
    }

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
        // cout << "    Output: " << val1 << endl;
        output = val1;
        iPointer += 2;
        return P_OUTPUT;
        break;
    // Jump if true:
    case 5:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        if (val1 != 0)
        {
            iPointer = val2;
        }
        else
        {
            iPointer += 3;
        }
        break;
    // Jump if false:
    case 6:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        if (val1 == 0)
        {
            iPointer = val2;
        }
        else
        {
            iPointer += 3;
        }
        break;
    // less than
    case 7:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        if (val1 < val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        iPointer += 4;
        break;
    // equals:
    case 8:
        val1 = getValue(data, iPointer + 1, paramModes[0]);
        val2 = getValue(data, iPointer + 2, paramModes[1]);
        storePos = data[iPointer + 3];
        if (val1 == val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        iPointer += 4;
        break;
    case 99:
        iPointer = data.size();
        return P_HALT;
        break;
    default:
        cerr << "Error: unknown opcode occured: " << opcode << endl;
        exit(1);
    }

    return P_RUN;
}

/**
 * Run a program until the end, instruction-by-instruction. Uses the given instruction pointer
 * for a specific program. Also UGLY!
 */
long runProgram(memory_t &data, memory_t::size_type ipNr)
{
    memory_t::size_type instructionPointer = instructionPointers[ipNr];
    ProgramResult res = P_RUN;
    while (res == P_RUN)
    {
        res = executeInstruction(instructionPointer, data);
        instructionPointers[ipNr] = instructionPointer;
    }
    return output;
}

void permute(vector<long> a, int l, int r, vector<vector<long>> &permutations)
{
    // Base case
    if (l == r)
        permutations.push_back(a);
    else
    {
        // Permutations made
        for (int i = l; i <= r; i++)
        {

            // Swapping done
            swap(a[l], a[i]);

            // Recursion called
            permute(a, l + 1, r, permutations);

            //backtrack
            swap(a[l], a[i]);
        }
    }
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
    readData<long>(args[1], ',', data);
    vector<long> inputs{5,6,7,8,9};
    vector<vector<long>> inputPermutations;
    vector<memory_t> ampPrograms;

    // Permute inputs:
    permute(inputs, 0, 4, inputPermutations);


    long highest = 0;
    // Test 1:
    // inputPermutations = vector<vector<long>>{{9,8,7,6,5}};
    // Test 2:
    // inputPermutations = vector<vector<long>>{{9,7,8,5,6}};
    for (auto actPermutation : inputPermutations)
    {
        // Build amps:
        ampPrograms.clear();
        instructionPointers.clear();
        inputDataPointer = 0;
        inputData[0] = 0;
        inputData[1] = 0;
        for (int i = 0; i < 5; i++)
        {
            ampPrograms.push_back(memory_t(data));
            instructionPointers.push_back(0);
        }
        // Run programs on each amp with feedback loop:
        unsigned long counter = 0;
        long i = 0;
        while (instructionPointers[ampPrograms.size()-1] < ampPrograms[ampPrograms.size()-1].size())
        {
            i = counter % ampPrograms.size(); // act amp, feedback loop counter
            if (counter < ampPrograms.size()) {
                inputDataPointer = 0;
                inputData[0] = actPermutation[i];
                inputData[1] = runProgram(ampPrograms[i], i);
            } else {
                inputDataPointer = 1;
                inputData[1] = runProgram(ampPrograms[i], i);
            }
            counter++;
        }
        if (inputData[1] > highest) {
            highest = inputData[1];
        }
    }
    cout << "Solution 2: Highest signal: " << highest << endl;
}

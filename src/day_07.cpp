#include <iostream>
#include <vector>
#include <queue>
#include "common.h"

using namespace std;

/**
 * Day 7 Part 2 - Amplification Circuit
 *
 * I introduced a Program class here, to keep all the program related
 * information (data, instruction pointer, input values) in one place for
 * each AMP (= Program). This makes things a lot easier.
 */

typedef vector<long> memory_t;

/**
 * An AMP can be in one of the following states:
 * - P_RUN: still running
 * - P_OUTPUT: Program has generated an output, and is halted until continued
 * - P_HALT: The program is done, no furter run
 */
enum ProgramResult
{
    P_RUN,
    P_OUTPUT,
    P_HALT
};

class Program
{
public:
    int pNr = 0;
    memory_t *memory;
    memory_t::size_type iPointer;
    queue<long> inputValues;
    long output;
    ProgramResult state;

    Program(memory_t *mem)
    {
        memory = new memory_t(*mem);
        iPointer = 0;
        state = P_RUN;
    }

    ~Program()
    {
        memory->clear();
        delete memory;
    }
};

/**
 * extracts the param modes from the whole opcode number.
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
 * Executes a single instruction (the next according to the Program's iPointer),
 * sets the output value (if applicable) and sets the iPointer to the next program
 * address.
 */
void executeNextInstruction(Program *program)
{
    // Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
    // "10102": --> "101" are param modes, "02" is the opcode 2.
    // The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
    // the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
    memory_t &data = *(program->memory);
    long opcode = data[program->iPointer] % 100; // last 2 digits are the opcode
    // cout << "I:" << iPointer << ": OP:" << opcode << "(" << data[iPointer] << ")" << endl;

    // cout << "  opcode: " << opcode;
    if (program->iPointer >= data.size())
    {
        program->state = P_HALT;
        return;
    }

    vector<int> paramModes{0, 0, 0};
    readParamModesFromOpcode(data[program->iPointer], paramModes);

    long val1;
    long val2;
    long storePos;
    long inputValue;
    switch (opcode)
    {
    // Add instr:
    case 1:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        storePos = data[program->iPointer + 3];
        data[storePos] = val1 + val2;
        program->iPointer += 4;
        break;
    // Multiply instr:
    case 2:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        storePos = data[program->iPointer + 3];
        data[storePos] = val1 * val2;
        program->iPointer += 4;
        break;
    // Input instr.
    case 3:
        inputValue = program->inputValues.front();
        program->inputValues.pop();
        storePos = data[program->iPointer + 1];
        data[storePos] = inputValue;
        // cout << "    Input value taken: " << inputValue << ", store at: " << storePos << endl;
        program->iPointer += 2;
        break;
    // Output instr.
    case 4:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        // cout << "    Output: " << val1 << endl;
        program->output = val1;
        program->iPointer += 2;
        program->state = P_OUTPUT;
        break;
    // Jump if true:
    case 5:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        if (val1 != 0)
        {
            program->iPointer = val2;
        }
        else
        {
            program->iPointer += 3;
        }
        break;
    // Jump if false:
    case 6:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        if (val1 == 0)
        {
            program->iPointer = val2;
        }
        else
        {
            program->iPointer += 3;
        }
        break;
    // less than
    case 7:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        storePos = data[program->iPointer + 3];
        if (val1 < val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        program->iPointer += 4;
        break;
    // equals:
    case 8:
        val1 = getValue(data, program->iPointer + 1, paramModes[0]);
        val2 = getValue(data, program->iPointer + 2, paramModes[1]);
        storePos = data[program->iPointer + 3];
        if (val1 == val2)
        {
            data[storePos] = 1;
        }
        else
        {
            data[storePos] = 0;
        }
        program->iPointer += 4;
        break;
    case 99:
        program->iPointer = data.size();
        program->state = P_HALT;
        break;
    default:
        cerr << "Error: unknown opcode occured: " << opcode << endl;
        exit(1);
    }
    // cout << endl;
}

/**
 * Run a program until the end, instruction-by-instruction, until it is set to
 * output or halt.
 */
void runProgram(Program *program)
{
    program->state = P_RUN;
    while (program->state == P_RUN)
    {
        // cout << "P:" << program->pNr << endl;
        executeNextInstruction(program);
        // cout << "E:" << program->pNr <<  ", state: " << program->state << endl;
    }
    // cout << "Exit: " << program->pNr << endl;
}

/**
 * Create all permutations of the given vector
 */
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

void runSolution1(memory_t &data)
{
    vector<long> inputs{0, 1, 2, 3, 4};
    vector<vector<long>> inputPermutations;
    vector<Program *> ampPrograms;

    // Permute inputs:
    permute(inputs, 0, 4, inputPermutations);

    long highest = 0;
    // Test 1:
    // inputPermutations = vector<vector<long>>{{4,3,2,1,0}};
    // Test 2:
    // inputPermutations = vector<vector<long>>{{0,1,2,3,4}};
    // Test 3:
    // inputPermutations = vector<vector<long>>{{1,0,4,3,2}};
    for (auto actPermutation : inputPermutations)
    {
        // Build amps:
        for (auto p : ampPrograms)
        {
            delete p;
        }
        ampPrograms.clear();

        // Create 5 new amp programs:
        for (int i = 0; i < 5; i++)
        {
            Program *p = new Program(&data);
            p->inputValues.push(actPermutation[i]);
            p->pNr = i + 1;
            ampPrograms.push_back(p);
        }
        // Run programs on each amp:
        long inputValue = 0;
        for (vector<long>::size_type i = 0; i < ampPrograms.size(); i++)
        {
            ampPrograms[i]->inputValues.push(inputValue);
            runProgram(ampPrograms[i]);
            inputValue = ampPrograms[i]->output;
        }
        if (inputValue > highest)
        {
            highest = inputValue;
        }
    }
    cout << "Solution 1: Highest signal: " << highest << endl;
}

void runSolution2(memory_t &data)
{
    vector<long> inputs{5, 6, 7, 8, 9};
    vector<vector<long>> inputPermutations;
    vector<Program *> ampPrograms;

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
        for (auto p : ampPrograms)
        {
            delete p;
        }
        ampPrograms.clear();

        // Create 5 new amp programs:
        for (int i = 0; i < 5; i++)
        {
            Program *p = new Program(&data);
            p->inputValues.push(actPermutation[i]);
            p->pNr = i + 1;
            if (i == 0)
            {
                p->inputValues.push(0);
            }
            ampPrograms.push_back(p);
        }

        // Run programs on each amp with feedback loop:
        unsigned long i = 0;
        unsigned long nextAmpIndex = 0;
        Program *actAmp;
        Program *nextAmp;
        while (ampPrograms.back()->state != P_HALT)
        {
            i = i % ampPrograms.size(); // act amp, feedback loop counter
            nextAmpIndex = (i + 1) % ampPrograms.size();
            actAmp = ampPrograms[i];
            nextAmp = ampPrograms[nextAmpIndex];
            runProgram(actAmp);
            // Feed the output of the act amp to the input of the next amp:
            nextAmp->inputValues.push(actAmp->output);
            i++;
        }
        if (ampPrograms.back()->output > highest)
        {
            highest = ampPrograms.back()->output;
        }
    }
    std::cout << "Solution 2: Highest signal: " << highest << endl;
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

    runSolution1(data);
    runSolution2(data);
}

#include <iostream>
#include <vector>
#include <map>
#include <queue>
#include "common.h"
#include <boost/multiprecision/cpp_int.hpp>
#include "CImg.h"

using namespace std;
using namespace boost::multiprecision;
using namespace boost;
using namespace cimg_library;

/**
 * Day 11 - Space Police
 */

/**
 * Memory is a map of memory address(key) and value (value):
 */
typedef map<cpp_int, cpp_int> memory_t;

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
    cpp_int iPointer;
    queue<cpp_int> inputValues;
    cpp_int output;
    ProgramResult state;
    cpp_int relativeBase;

    Program(memory_t *mem)
    {
        memory = new memory_t(*mem);
        iPointer = cpp_int(0);
        relativeBase = cpp_int(0);
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
void readParamModesFromOpcode(cpp_int opcode, vector<int> &modes)
{
    string str = lexical_cast<string>(opcode);
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
            case '2':
                modes[i] = 2;
                break;
            }
        }
    }
    // cout << "  Param modes" << modes[0] << ":"<<modes[1] <<":"<<modes[2] << endl;
}

/**
 * Reads data at the given pointer address, initializes it with zero if that
 * address is not yet initialized.
 */
cpp_int readValueFromMemory(memory_t &data, cpp_int iPointer)
{
    memory_t::iterator it = data.find(iPointer);
    if (it != data.end()) {
        return it->second;
    } else {
        data[iPointer] = cpp_int(0);
        return data[iPointer];
    }
}

/**
 * Reads a value from memory:
 * - iPointer is the address of the parameter to read.
 * - The read parameter is either a direct value or a pointer, depending on the param's opmode.
 *
 * if paramMode is 0, this means position mode: The value at iPointer is a memory address to the actual value
 * if paramMode is 1, this means immediate mode: The value at iPointer is the actual value
 * if paramMode is 2, this means relative position mode: like position mode, but with a relative offset
 */
cpp_int getValue(Program* program, cpp_int iPointer, int paramMode)
{
    memory_t &data = *(program->memory);
    cpp_int paramValue = data[iPointer];
    if (paramMode == 0)
    {
        // position mode: data[iPointer] is a position pointer
        return readValueFromMemory(data, paramValue);
    }
    else if (paramMode == 1)
    {
        // immediate mode: data[iPointer] is the value
        return paramValue;
    } else if (paramMode == 2) {
        // relative mode: like position mode, but with a relative offset:
        return readValueFromMemory(data, paramValue + program->relativeBase);
    }
    else
    {
        cerr << "Unknown param mode: " << paramMode << endl;
        exit(1);
    }
}

/**
 * Calculates the store address for a write operation.
 *
 * iPointer is the address of a parameter to read. Depending on the paramMode, this is either
 * an absolute value or must be offset by the actual relativeBase.
 */
cpp_int getStorePos(Program* program, cpp_int iPointer, int paramMode) {
    // No immediate mode for writing.
    memory_t &data = *(program->memory);
    cpp_int paramValue = data[iPointer];
    if (paramMode == 2) {
        // relative mode: memory address with offset
        return paramValue + program->relativeBase;
    } else {
        return paramValue;
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
    int opcode = static_cast<int>(data[program->iPointer] % 100); // last 2 digits are the opcode
    // cout << "I:" << program->iPointer << ": OP:" << opcode << "(" << data[program->iPointer] << ")" << endl;

    vector<int> paramModes{0, 0, 0};
    readParamModesFromOpcode(data[program->iPointer], paramModes);

    cpp_int val1;
    cpp_int val2;
    cpp_int storePos;
    cpp_int inputValue;
    switch (opcode)
    {
    // Add instr:
    case 1:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        data[storePos] = val1 + val2;
        program->iPointer += 4;
        break;
    // Multiply instr:
    case 2:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
        data[storePos] = val1 * val2;
        program->iPointer += 4;
        break;
    // Input instr.
    case 3:
        inputValue = program->inputValues.front();
        program->inputValues.pop();
        storePos = getStorePos(program, program->iPointer + 1, paramModes[0]);
        data[storePos] = inputValue;
        // cout << "    Input value taken: " << inputValue << ", store at: " << storePos << endl;
        program->iPointer += 2;
        break;
    // Output instr.
    case 4:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        // cout << "    Output: " << val1 << endl;
        program->output = val1;
        program->iPointer += 2;
        program->state = P_OUTPUT;
        break;
    // Jump if true:
    case 5:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
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
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
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
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
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
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        val2 = getValue(program, program->iPointer + 2, paramModes[1]);
        storePos = getStorePos(program, program->iPointer + 3, paramModes[2]);
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
    // modify relative base:
    case 9:
        val1 = getValue(program, program->iPointer + 1, paramModes[0]);
        program->relativeBase += val1;
        program->iPointer += 2;
        break;
    case 99:
        // program->iPointer = data.size();
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

void start(memory_t &data, cpp_int startInput, const string& imgName) {
    Program program(&data);
    map<string,cpp_int> paintArea;
    char dirs[] = {'U', 'R', 'D', 'L'};
    int dirIndex = 0;
    int coordX = 0, coordY = 0;
    int minX = INT_MAX, maxX = INT_MIN, minY = INT_MAX, maxY = INT_MIN;
    cpp_int actInput;
    bool firstRun = true;
    // program.inputValues.push(cpp_int(1));
    while (program.state != P_HALT) {
        // read the input (color) on the actual coords.
        // 0 / not set means black, 1 means white
        string coordKey = to_string(coordX) + ":" + to_string(coordY);
        map<string,cpp_int>::iterator findIt = paintArea.find(coordKey);
        if (findIt == paintArea.end()) {
            if (firstRun) {
                actInput = startInput;
            } else {
                actInput = cpp_int(0);
            }
        } else {
            actInput = findIt->second;
        }
        firstRun = false;
        // fill the actual color as program input, execute:
        program.inputValues.push(actInput);
        runProgram(&program);
        if (program.state == P_OUTPUT) {
            // 1st output is the color to be painted:
            actInput = program.output;
            paintArea[coordKey] = actInput;
            if (coordX < minX) minX = coordX;
            if (coordX > maxX) maxX = coordX;
            if (coordY < minY) minY = coordY;
            if (coordY > maxY) maxY = coordY;
        } else {
            cout << "End of program" << endl;
            break;
        }
        // start again, wait for 2nd output, the dir:
        runProgram(&program);
        if (program.state == P_OUTPUT) {
            // 2nd output is the dir to turn:
            actInput = program.output;
            if (actInput == 0) {
                // turn left
                dirIndex = (dirIndex + 3) % 4;
            } else if (actInput == 1) {
                // turn right
                dirIndex = (dirIndex + 1) % 4;
            } else {
                cerr << "Oops, unknown direction output! die!" << endl; exit(1);
            }
            // calc new coords:
            if (dirs[dirIndex] == 'U') {
                coordY--;
            }
            if (dirs[dirIndex] == 'R') {
                coordX++;
            }
            if (dirs[dirIndex] == 'D') {
                coordY++;
            }
            if (dirs[dirIndex] == 'L') {
                coordX--;
            }
        } else {
            cerr << "Oops, premature end of program! Not good!" << endl; exit(1);
        }

    }
    int dX = maxX - minX;
    int dY = maxY - minY;
    // cout << dX << ":" << dY << endl; exit(1);
    CImg<unsigned char> pngImg(dX+4, dY+4, 1, 3, 0); // add a small border of 2 px per side
    for (int y = minY; y <= maxX; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            string coordKey = to_string(x) + ":" + to_string(y);
            map<string,cpp_int>::iterator findIt = paintArea.find(coordKey);
            if (findIt != paintArea.end() && findIt->second == 1) {
                // cout <<x-minX+2 << ":" <<  y-minY+2 << endl;
                cout << "*";
                pngImg(x-minX+2, y-minY+2, 0, 0) = 255;
                pngImg(x-minX+2, y-minY+2, 0, 1) = 255;
                pngImg(x-minX+2, y-minY+2, 0, 2) = 255;
            } else {
                cout << " ";
            }
        }
        cout << endl;
    }
    cout << "Number of painted tiles: " << paintArea.size() << endl;
    pngImg.save_png(imgName.c_str());
    cout << "Output image saved to: " << imgName << endl;
}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<long> fileData;
    memory_t data;
    readData<long>(args[1], ',', fileData);

    // convert input to big precision map:
    for (vector<long>::size_type i = 0; i < fileData.size(); ++i) {
        data[cpp_int(i)] = cpp_int(fileData[i]);
    }

    memory_t run1_data(data);
    memory_t run2_data(data);
    // Solution 1:
    cout << "Solution 1" << endl;
    start(run1_data, cpp_int(0), "day_11_solution1.png");
    cout << endl <<endl << "Solution 2" << endl;
    // Solution 2:
    start(run2_data, cpp_int(1), "day_11_solution2.png");
}

#ifndef INTCODE_H
#define INTCODE_H

#include <map>
#include <vector>
#include <map>
#include <queue>
#include "common.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <thread>
#include <chrono>

using namespace boost::multiprecision;
using namespace boost;
/**
 * The INTCODE Computer and helper structures, used in a variety of puzzles this year.
 *
 * The Program class represents the Intcode processor.
 * Load a memory structure (memory_t) on constructing,
 * and execute runProgram().
 *
 * When runProgram returns, the program is in two possible modes:
 *
 * - P_HALT: The program has ended (reached opcode 99)
 * - P_OUTPUT: The program has produced an output (reached opcode 4)
 *
 * After that, you can:
 *
 * - read the output with Program.output
 * - continue the program (program.runProgram() again)
 *
 * To provide input values, use
 *
 * program.inputValues.push(value)
 */

/**
 * Memory is a map of memory address(key) and value (value):
 */
typedef map<cpp_int, cpp_int>
    memory_t;

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

/**
 * Reads an Intcode program from the given filename, into the given memory
 */
void readIntcodeProgramFromFile(const string &filename, memory_t &memory)
{
    // Read input file:
    vector<long> fileData;
    readData<long>(filename.c_str(), ',', fileData);

    // convert input to big precision map:
    for (vector<long>::size_type i = 0; i < fileData.size(); ++i)
    {
        memory[cpp_int(i)] = cpp_int(fileData[i]);
    }
}

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

        void asciiInput(const string& input) {
            for (auto c : input) {
                this->inputValues.push(cpp_int(c));
            }
            this->inputValues.push(cpp_int('\n'));
        }

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
        }

        /**
         * Reads data at the given pointer address, initializes it with zero if that
         * address is not yet initialized.
         */
        cpp_int readValueFromMemory(cpp_int iPointer)
        {
            memory_t::iterator it = this->memory->find(iPointer);
            if (it != this->memory->end())
            {
                return it->second;
            }
            else
            {
                ( *(this->memory) )[iPointer] = cpp_int(0);
                return ( *(this->memory) )[iPointer];
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
        cpp_int getValue(cpp_int iPointer, int paramMode)
        {
            cpp_int paramValue = (*(this->memory))[iPointer];
            if (paramMode == 0)
            {
                // position mode: memory[iPointer] is a position pointer
                return this->readValueFromMemory(paramValue);
            }
            else if (paramMode == 1)
            {
                // immediate mode: memory[iPointer] is the value
                return paramValue;
            }
            else if (paramMode == 2)
            {
                // relative mode: like position mode, but with a relative offset:
                return this->readValueFromMemory(paramValue + this->relativeBase);
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
         * iPointer is the address of a parameter to write. Depending on the paramMode, this is either
         * an absolute value or must be offset by the actual relativeBase.
         */
        cpp_int getStorePos(cpp_int iPointer, int paramMode)
        {
            // No immediate mode for writing.
            memory_t &data = *(this->memory);
            cpp_int paramValue = data[iPointer];
            if (paramMode == 2)
            {
                // relative mode: memory address with offset
                return paramValue + this->relativeBase;
            }
            else
            {
                return paramValue;
            }
        }

        virtual cpp_int getInputValue() {
            cpp_int val = this->inputValues.front();
            this->inputValues.pop();
            return val;
        }

        /**
         * Executes a single instruction (the next according to the Program's iPointer),
         * sets the output value (if applicable) and sets the iPointer to the next program
         * address.
         */
        void executeNextInstruction()
        {
            // Opcode: The last 2 digits are the opcode, the rest is param modes, e.g.:
            // "10102": --> "101" are param modes, "02" is the opcode 2.
            // The param modes are read right-to-left: the most right mode digit is the mode for the 1st param,
            // the next to the right is the param mode for the 2nd param and so on (so it's the reverse direction of the params, neat :-))
            memory_t &data = *(this->memory);
            int opcode = static_cast<int>(data[this->iPointer] % 100); // last 2 digits are the opcode

            vector<int> paramModes{0, 0, 0};
            this->readParamModesFromOpcode(data[this->iPointer], paramModes);

            cpp_int val1;
            cpp_int val2;
            cpp_int storePos;
            cpp_int inputValue;
            switch (opcode)
            {
                // Add instr:
                case 1:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    storePos = this->getStorePos(this->iPointer + 3, paramModes[2]);
                    data[storePos] = val1 + val2;
                    this->iPointer += 4;
                    break;
                    // Multiply instr:
                case 2:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    storePos = this->getStorePos(this->iPointer + 3, paramModes[2]);
                    data[storePos] = val1 * val2;
                    this->iPointer += 4;
                    break;
                    // Input instr.
                case 3:
                    inputValue = this->getInputValue();
                    storePos = this->getStorePos(this->iPointer + 1, paramModes[0]);
                    data[storePos] = inputValue;
                    this->iPointer += 2;
                    break;
                    // Output instr.
                case 4:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    this->output = val1;
                    this->iPointer += 2;
                    this->state = P_OUTPUT;
                    break;
                    // Jump if true:
                case 5:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    if (val1 != 0)
                    {
                        this->iPointer = val2;
                    }
                    else
                    {
                        this->iPointer += 3;
                    }
                    break;
                    // Jump if false:
                case 6:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    if (val1 == 0)
                    {
                        this->iPointer = val2;
                    }
                    else
                    {
                        this->iPointer += 3;
                    }
                    break;
                    // less than
                case 7:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    storePos = getStorePos(this->iPointer + 3, paramModes[2]);
                    if (val1 < val2)
                    {
                        data[storePos] = 1;
                    }
                    else
                    {
                        data[storePos] = 0;
                    }
                    this->iPointer += 4;
                    break;
                    // equals:
                case 8:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    val2 = this->getValue(this->iPointer + 2, paramModes[1]);
                    storePos = getStorePos(this->iPointer + 3, paramModes[2]);
                    if (val1 == val2)
                    {
                        data[storePos] = 1;
                    }
                    else
                    {
                        data[storePos] = 0;
                    }
                    this->iPointer += 4;
                    break;
                    // modify relative base:
                case 9:
                    val1 = this->getValue(this->iPointer + 1, paramModes[0]);
                    this->relativeBase += val1;
                    this->iPointer += 2;
                    break;
                case 99:
                    this->state = P_HALT;
                    break;
                default:
                    cerr << "Error: unknown opcode occured: " << opcode << endl;
                    exit(1);
            }
            // cout << endl;
        }


        /**
         * Run the program, instruction-by-instruction, until it is set to
         * output or halt.
         */
        void runProgram()
        {
            this->state = P_RUN;
            while (this->state == P_RUN)
            {
                this->executeNextInstruction();
            }
        }

        virtual ~Program()
        {
            memory->clear();
            delete memory;
        }
};

#endif

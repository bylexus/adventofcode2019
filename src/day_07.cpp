#include <iostream>
#include <vector>
#include <queue>
#include "common.h"
#include "intcode.h"

using namespace std;

/**
 * Day 7 Part 2 - Amplification Circuit
 */


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

    cpp_int highest(0);
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
        cpp_int inputValue(0);
        for (vector<long>::size_type i = 0; i < ampPrograms.size(); i++)
        {
            ampPrograms[i]->inputValues.push(inputValue);
            (ampPrograms[i])->runProgram();
            inputValue = ampPrograms[i]->output;
        }
        if (inputValue > highest)
        {
            highest = inputValue;
        }
    }
    for (auto p : ampPrograms)
    {
        delete p;
    }
    ampPrograms.clear();
    cout << "Solution 1: Highest signal: " << highest << endl;
}

void runSolution2(memory_t &data)
{
    vector<long> inputs{5, 6, 7, 8, 9};
    vector<vector<long>> inputPermutations;
    vector<Program *> ampPrograms;

    // Permute inputs:
    permute(inputs, 0, 4, inputPermutations);

    cpp_int highest(0);
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
            actAmp->runProgram();
            // Feed the output of the act amp to the input of the next amp:
            nextAmp->inputValues.push(actAmp->output);
            i++;
        }
        if (ampPrograms.back()->output > highest)
        {
            highest = ampPrograms.back()->output;
        }
    }
    for (auto p : ampPrograms)
    {
        delete p;
    }
    ampPrograms.clear();
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
    readIntcodeProgramFromFile(args[1], data);

    runSolution1(data);
    runSolution2(data);
}

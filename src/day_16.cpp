#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <algorithm>
#include "common.h"

using namespace std;

/**
 * Day 16
 *
 */

vector<vector<int>> patterns;


/**
 * For each element, a repeating pattern is created.
 * Start pattern is 0,1,0,-1
 * - Pattern for 1st nr is: 0,1,0,-1
 * - patttern for 2nd nr is: 0,0,1,1,0,0,-1,-1
 * - patttern for 3nd nr is: 0,0,0,1,1,1,0,0,0,-1,-1,-1
 */
void buildPatterns(unsigned int nrOfElements) {
    vector<int> basePattern{0,1,0,-1};
    for (unsigned int i = 0; i < nrOfElements; i++) {
        vector<int>pattern;
        for (unsigned int pIndex = 0; pIndex < basePattern.size(); pIndex++) {
            int actNr = basePattern[pIndex];
            for (unsigned int repeat = 0; repeat <= i; repeat++) {
                pattern.push_back(actNr);
            }
        }
        patterns.push_back(pattern);
    }
}

void calcPhase(vector<int> &elements) {
    // Loop over all input numbers
    for (unsigned int nrIndex = 0; nrIndex < elements.size(); nrIndex++ ) {
        int sum = 0;
        for (unsigned int i = 0; i < elements.size(); i++) {
            // for each number, calc nr * pattern-nr, while pattern-nr is offset by 1.
            // skip first nr in offset pattern.
            vector<int> pattern = patterns[nrIndex];
            sum += elements[i] * pattern[(i+1) % pattern.size()];
        }
        elements[nrIndex] = abs(sum%10);
    }
}

void calcPhases(unsigned int nr, vector<int> &elements) {
    while (nr > 0) {
        calcPhase(elements);
        nr--;
    }
}

void outputElements(vector<int>&elements, unsigned long amount) {
    amount = min(elements.size(), amount);
    for (unsigned int i = 0; i < amount; i++) {
        cout << elements[i];
    }
    cout << endl;
}

void outputElements(vector<int>&elements) {
    outputElements(elements, elements.size());
}


int main(int argc, char *args[])
{
    bool display = argc >= 3 && string("--display").compare(args[2]) == 0 ? true : false;
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> fileData;
    readLines(args[1], fileData);

    string input = fileData[0];
    vector<int>elements;
    for (auto i : input) {
        elements.push_back(i - '0');
    }
    cout << "Elements: " << elements.size() << endl;

    buildPatterns(elements.size());
    // for (auto p : patterns) {
    //     for (auto e : p) {
    //         cout << e << ",";
    //     }
    //     cout << endl;
    // }

    calcPhases(100, elements);
    // outputElements(elements);

    cout << "Solution 1: First 8 digits after 100 rounds: ";
    outputElements(elements, 8);

}

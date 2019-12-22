#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "common.h"

using namespace std;

/**
 * Day 22 - Slam Shuffle
 *
 * Part 1 - easy, but I guess that will not work for part 2
 * Part 2 - as I thought - not possible with the naive solution from part 1
 *          Another detective puzzle, I guess. Some number inspection needs to be done here.
 */

int deckSize = 10007;
typedef vector<int> deck_t;
deck_t* deck = new deck_t(deckSize);
deck_t* tempDeck = new deck_t(deckSize);

void swapDecks() {
    deck_t* tmp = deck;
    deck = tempDeck;
    tempDeck = tmp;
}

void fillDeck() {
    for (auto i = 0; i < deckSize; i++) {
        (*deck)[i] = i;
    }
}

void dealIntoNewStack() {
    for (auto i = 0; i < deckSize; ++i) {
        ( *tempDeck )[i] = (*deck)[deckSize - i - 1];
    }
    swapDecks();
}

void cutNCards(int n) {
    int offset = (deckSize + n) % deckSize;
    for (auto i = 0; i < deckSize; ++i) {
        (*tempDeck)[i] = (*deck)[(i + offset) % deckSize];
    }
    swapDecks();
}

void dealWithIncrement(int n) {
    int offset = 0;
    for (auto i = 0; i < deckSize; ++i) {
        offset = (i * n) % deckSize;
        (*tempDeck)[offset] = (*deck)[i];
    }
    swapDecks();
}


void printDeck() {
    for (auto i = 0; i < deckSize; ++i) {
        cout << (*deck)[i] << " ";
    }
    cout << endl;
}

void applyCommands(vector<string> &commands)
{
    for (auto line : commands) {
        if (line == "deal into new stack") {
            dealIntoNewStack();
        } else if (line.substr(0,3) == "cut") {
            int n = stoi(line.substr(4));
            cutNCards(n);
        } else if (line.substr(0,9) == "deal with") {
            int n = stoi(line.substr(20));
            dealWithIncrement(n);
        }
    }
}

void solution1(vector<string> &commands) {
    fillDeck();
    applyCommands(commands);

    // Now, search card # 2019's pos:
    for (int i = 0; i < deckSize; ++i) {
        if ((*deck)[i] == 2019) {
            cout << "Solution 1: Card 2019 has position: " << i << endl;
        }
    }
}

void solution2(vector<string> &commands) {
    fillDeck();
    for (int i = 0; i < 1000; i++) {
        applyCommands(commands);
        cout << (*deck)[2020] << endl;
    }
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
    vector<string> inputLines;
    readLines(args[1],  inputLines);


    solution1(inputLines);

    solution2(inputLines);
}

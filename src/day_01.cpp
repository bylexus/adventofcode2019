#include <iostream>
#include <sstream>
#include <fstream>
#include <math.h>

using namespace std;

/**
 * Calculate the total amount of fuel needed for the spacecraft to launch!
 *
 * Fuel required to launch a given module is based on its mass.
 * Specifically, to find the fuel required for a module, take its mass,
 * divide by three, round down, and subtract 2.
 *
 * Problem 1:
 * Sum them all up, and you have the amount of fuel.
 *
 * Problem 2:
 * The fuel itself has a mass, so caluclate how much fuel is needed WITH fuel... recursively,
 * until the outcoming mass is <= 0
 */

long calcFuel(long mass) {
    return floor(mass / 3.0) - 2;
}

/**
 * 2nd part: calc the fuel needed to transport fuel also:
 *
 * Take the fuel output as mass, too, calculate until the outcoming
 * fuel is <= 0:
 */
long calcFuelRecursive(long mass) {
    long fuel = calcFuel(mass);
    if (fuel > 0) {
        return fuel + calcFuelRecursive(fuel);
    } else return 0;
}

int main()
{
    ifstream infile("../src/day_01-input.txt");
    string line;
    long nr;
    long fuelSum = 0;
    long fuelWithFuelSum = 0;

    while(getline(infile, line)) {
        istringstream(line) >> nr;
        fuelSum += (nr > 0 ? calcFuel(nr) : 0);
        fuelWithFuelSum += (nr > 0 ? calcFuelRecursive(nr) : 0);
    }
    cout << "Solution 1: Fuel needed: " << fuelSum << endl;
    cout << "Solution 2: Fuel needed: with fuel mass: " << fuelWithFuelSum << endl;
}

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include "common.h"
#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/cpp_int.hpp>

using namespace std;
using namespace boost::multiprecision;

/**
 * Day 14 - Space Stoichiometry
 *
 * NOTE: The function "calcOre" is WAYS too slow - somewhere I have a calculation error.
 * I will fix this when I find time.
 */
class Chemical;

map<string, Chemical *> chemicalMap;

cpp_int allOre(1000000000000);
cpp_int totalOre(1000000000000);

class Chemical
{
public:
    string name;
    cpp_int amount = 0;
    cpp_int waste = 0;
    vector<Chemical *> needs;
};

Chemical *createChemical(string &str)
{
    vector<string> parts;
    split(str, ' ', parts);
    Chemical *c = new Chemical();
    c->name = parts[1];
    c->amount = cpp_int(parts[0]);
    return c;
}

cpp_int calcOre(Chemical *input)
{
    // Given the input chemical, calculate the amount of ore
    // needed recursively for all dependant chems
    cpp_int oreAmount(0);
    // cout << input->name << " needs: ";
    for (auto chem : input->needs)
    {
        // cout << "  " << chem->amount << " of " << chem->name << endl;
        if (chem->name.compare("ORE") == 0)
        {
            oreAmount += chem->amount;
            totalOre -= chem->amount;
            // cout << "Taking " << chem->amount << " of ORE" << endl;
        }
        else
        {
            cpp_int target = chem->amount;

            while (target > 0)
            {
                if (chemicalMap[chem->name]->waste > 0)
                {
                    target -= chemicalMap[chem->name]->waste;
                    chemicalMap[chem->name]->waste = 0;
                }
                else
                {
                    target -= chemicalMap[chem->name]->amount;
                    oreAmount += calcOre(chemicalMap[chem->name]);
                }
                // cout << "Taking " << chemicalMap[chem->name]->amount << " of " << chem->name << endl;
            }
            if (target < 0)
            {
                chemicalMap[chem->name]->waste = target * -1;
            }
        }
    }
    // cout << endl;
    return oreAmount;
}

void resetWaste()
{
    for (map<string, Chemical *>::iterator it = chemicalMap.begin(); it != chemicalMap.end(); ++it)
    {
        it->second->waste = 0;
    }
}

cpp_int orePerFuel(cpp_int fuel) {
    cpp_int sum = 0;
    resetWaste();
    Chemical* startChem = new Chemical();
    Chemical* fuelsChem = new Chemical();
    fuelsChem->name = "FUEL";
    fuelsChem->amount = fuel;
    startChem->needs.push_back(fuelsChem);
    return calcOre(startChem);
}

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> fileData;
    readLines(args[1], fileData);

    // Build chemical map:
    for (auto token : fileData)
    {
        vector<string> parts;
        split(token, '>', parts);
        string last = trim(parts.back());
        Chemical *target = createChemical(last);
        cout << "Target chem: " << target->name << ": " << target->amount << "needs: ";
        for (unsigned int i = 0; i < parts.size() - 1; i++)
        {
            string part = parts[i];
            boost::algorithm::erase_all(part, "=");
            vector<string> singleToken;
            split(part, ',', singleToken);
            for (auto singlePart : singleToken)
            {
                singlePart = trim(singlePart);
                Chemical *c = createChemical(singlePart);
                target->needs.push_back(c);
                cout << c->amount << " " << c->name << ", ";
            }
            cout << endl;
        }
        chemicalMap[target->name] = target;
    }

    // Begin with fuel, calculate recursively how many ore we need:
    cpp_int maxOreForFuel = orePerFuel(1);
    cout << "Solution 1: Ore needed for one Unit of fuel: " << maxOreForFuel << endl;
    // exit(1);

    cpp_int maxFuel = allOre / maxOreForFuel;
    cout << "We can produce maximum " << maxFuel << " Units of fuel" << endl;

    // 2nd part: binary search:
    // Now we have a function y = f(x), while x is the amount of fuel, and y is the needed ore.
    // So we have a typical binary search situation: we know y (1 trillion), and look for x:

    cpp_int min_f = allOre / maxOreForFuel;
    cpp_int max_f = 2*min_f;
    while (max_f > min_f+1) {
        cpp_int prod_fuel = min_f + (max_f - min_f) / 2;
        if (orePerFuel(prod_fuel) > allOre) {
            max_f = prod_fuel;
        } else {
            min_f = prod_fuel;
        }
    }

    cout << "Solution Part 2: " << min_f << " fuel" << endl;
}

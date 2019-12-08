#include <iostream>
#include <vector>
#include "common.h"

using namespace std;

/**
 * Day 8 - Space Image Format
 */

typedef vector<long> memory_t;

int main(int argc, char *args[])
{
    if (argc < 2)
    {
        cerr << "Error: give input file on command line" << endl;
        exit(1);
    }

    // Read input file:
    vector<string> data;
    readData<string>(args[1], '\n', data);
    string img = data[0];

    unsigned int height = 6;
    unsigned int width = 25;
    // unsigned int layers = img.length() / (height * width);
    char imagechar[height][width];

    cout << "Layers in img: " << img.length() / (6 * 25) << endl;

    unsigned int layer = 0;
    unsigned int index = 0;
    unsigned int maxZeroPx = 0;
    unsigned int solution = 0;
    unsigned int zeroPx = 0;
    unsigned int onePx = 0;
    unsigned int twoPx = 0;
    while (index < img.length())
    {
        zeroPx = 0;
        onePx = 0;
        twoPx = 0;
        for (unsigned int y = 0; y < height; ++y)
        {
            for (unsigned int x = 0; x < width; ++x)
            {
                char px = img[index];
                if (layer > 0)
                {
                    char pxAbove = imagechar[y][x];
                    if (pxAbove == '2')
                    {
                        // above is transparent, so my pixel will win!
                        imagechar[y][x] = px;
                    }
                }
                else
                {
                    // I am the first:
                    imagechar[y][x] = px;
                }
                if (px == '0')
                {
                    zeroPx++;
                }
                if (px == '1')
                {
                    onePx++;
                }
                if (px == '2')
                {
                    twoPx++;
                }
                index++;
            }
        }
        if (maxZeroPx == 0 || maxZeroPx > zeroPx)
        {
            maxZeroPx = zeroPx;
            solution = onePx * twoPx;
        }
        layer++;
    }
    cout << "Solution 1: N° of 1px * N° of 2px: " << solution << endl;


    cout << "Solution 2:" << endl << endl;
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            cout << (imagechar[y][x] == '1' ? '*' : ' ');
        }
        cout << endl;
    }
}

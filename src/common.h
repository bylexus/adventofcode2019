#include <vector>

using namespace std;

/**
 * Read data from file - split by line/delimiter
 *
 * Fills a vector with all input data
 */
template<typename T>
void readData(char *filename, char delim, vector<T> &data);

void split(const string &str, char delim, vector<string> &data);

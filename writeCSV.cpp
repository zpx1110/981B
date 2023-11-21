#include "writeCSV.h"
#include "qtypes.h"

#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

writeCSV::writeCSV()
{

}

void writeCSV::writefile(quint64 data [15], bool NS, bool MS, const string& filename)
{
    ofstream file(filename, ios::app | ios::out);

    if (!file.is_open())
    {
        cerr << "Unable to open file: " << filename << endl;
        return;
    }

    int b = 0;
    for (int j=0; j<15; j++)
    {
        if (data [j] != 0)
            b++;
    }

    // write to CSV
    for (int i = 0; i < b; i++)
    {
        file << data[i];
        file << ",";
    }

    if (NS)
        file << "accept";
    else
        file << "reject";
    file << ",";

    if (MS)
        file << "accept";
    else
        file << "reject";

    file << "\n";

    file.close();
}

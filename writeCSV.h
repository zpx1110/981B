#ifndef WRITECSV_H
#define WRITECSV_H

#include <iostream>
#include <fstream>
#include <vector>
#include <QKeyEvent>

using namespace std;

class writeCSV
{
public:

    writeCSV();

    static void writefile(quint64 data [15], bool NS, bool MS, const string& filename);

};

#endif // WRITECSV_H

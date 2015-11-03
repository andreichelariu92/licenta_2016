#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include "FileOperations.h"
using namespace std;

int main()
{
    boost::filesystem::path testPath("./testfile");
    std::vector<char> buffer = {'a', 'b', 'c', 'd'};
    overWriteData(testPath, 5, buffer);
    return 0;
}

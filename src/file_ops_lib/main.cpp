#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include "FileOperations.h"
using namespace std;

int main()
{
    boost::filesystem::path testPath("/home/andrei/testfile1");

    FileOperationOptions option = directory;
    createFileOrDirectory(option, testPath);
    return 0;
}

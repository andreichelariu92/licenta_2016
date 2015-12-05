#include <iostream>
#include <fstream>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

#include "FileOperations.h"
using namespace std;

int main()
{
    boost::filesystem::path testPath("./tmp");

    FileOperationOptions option = FOO_directory;
    deleteFileOrDirectory(testPath);
    return 0;
}

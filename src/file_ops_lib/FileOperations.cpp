#include "FileOperations.h"

#include <fstream>
#include <iostream>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/fstream.hpp>

static void s_verifyPathFile(const boost::filesystem::path& path)
{
    namespace fs = boost::filesystem;

    if ( !fs::exists(path) )
    {
        std::cout<<"The file "<<path.string()<<" does not exist\n";
        //throw exception
    }
    if ( !fs::is_regular_file(path) )
    {
        std::cout<<path.string()<<" is not a regular file\n";
        //throw exception
    }
}
static void s_verifyPositionFile(boost::filesystem::fstream& file, unsigned int position)
{
   file.seekg(0, file.end);
   unsigned int fileSize = file.tellg();
   file.seekg(0);
   if (fileSize < position)
   {
       std::cout<<position<<" is past the file end\n";
       //throw exception
   }
}
static void s_verifyStatusFile(const boost::filesystem::fstream& file, const boost::filesystem::path& filePath)
{
    if (file.bad())
    {
        std::cout<<"the operation on the file "<<filePath<<" failed\n";
        //throw exception
    }
}
static int s_getFileLength(boost::filesystem::fstream& file)
{
   file.seekg(0, file.end);
   unsigned int fileSize = file.tellg();
   file.seekg(0);
   return fileSize;
}

std::vector<char> readData(const boost::filesystem::path& path, unsigned int position, unsigned int size)
{
    //the verifying functions will
    //break to flow constrol if something wrong happens
    //the caller must catch all the exceptions
    namespace fs = boost::filesystem;

    s_verifyPathFile(path);
    fs::fstream file(path,  std::ios_base::binary | std::ios_base::in);

    s_verifyPositionFile(file, position);
    file.seekg(position);

    //alocate memory for the data
    std::vector<char> output(size);
    file.read(&output[0], size);
    s_verifyStatusFile(file, path);

    return output;
}
void overWriteData(const boost::filesystem::path& path, unsigned int position, const std::vector<char>& buffer)
{
    namespace fs = boost::filesystem;

    s_verifyPathFile(path);
    fs::fstream file(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

    s_verifyPositionFile(file, position);
    file.seekg(position);

    file.write(&buffer[0], buffer.size());

    s_verifyStatusFile(file, path);
}

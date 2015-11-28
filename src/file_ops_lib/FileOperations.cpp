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
void removeData(const boost::filesystem::path& path, unsigned int position, unsigned int length)
{
   namespace fs = boost::filesystem;

   s_verifyPathFile(path);
   fs::fstream file(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

   const unsigned int fileLength = s_getFileLength(file);
   //if the requested length to delete is too big
   //the newFileBuffer will hold just the first characters (position)
   s_verifyPositionFile(file, position);
   const unsigned int newFileBufferSize = (fileLength > length) ? (fileLength - length) : position;
   std::vector<char> newFileBuffer(newFileBufferSize);

   file.read(&newFileBuffer[0], position);
   s_verifyStatusFile(file, path);

   file.seekg(position + length);

   file.read(&newFileBuffer[position], newFileBufferSize - position);
   s_verifyStatusFile(file, path);

   file.close();
   file.open(path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
   file.write(&newFileBuffer[0], newFileBufferSize);

}
void addData(const boost::filesystem::path& path, unsigned int position, const std::vector<char>& buffer)
{
   namespace fs = boost::filesystem;

   s_verifyPathFile(path);
   fs::fstream file(path, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

   const unsigned int fileLength = s_getFileLength(file);
   std::vector<char> newFileBuffer(buffer.size() + fileLength);

   s_verifyPositionFile(file, position);
   file.read(&newFileBuffer[0], position);

   //after the first position characters from file
   //add the data from the buffer
   std::copy(buffer.begin(), buffer.end(), (newFileBuffer.begin()+position) );

   const unsigned int partialBufferSize = position + buffer.size();
   file.read(&newFileBuffer[partialBufferSize], fileLength - position);
   s_verifyStatusFile(file, path);

   //to overwrite the file,
   //we need to open it with std::ios_base::trunc flag
   file.close();
   file.open(path, std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
   file.write(&newFileBuffer[0], newFileBuffer.size());
}
void createFileDirectory(unsigned int position, const boost::filesystem::path& path)
{

}

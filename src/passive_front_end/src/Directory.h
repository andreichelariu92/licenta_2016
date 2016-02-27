#ifndef Directory_H_INCLUDE_GUARD
#define Directory_H_INCLUDE_GUARD

//standard library headers
#include <string>
#include <vector>
#include <exception>
//OS headers
#include <dirent.h>

///class that wraps around the
///DIR structure of the Operating System
class Directory
{
private:
    std::string path_;
    DIR* dirStructure_;
public:
    ///constructor
    ///if the path is not correct, it throws
    ///a DirectoryException
    Directory(std::string path);
    ///delete copy operations
    Directory(const Directory& source) = delete;
    Directory& operator=(const Directory& source) = delete;
    ///move operations
    Directory(Directory&& source);
    Directory& operator=(Directory&& source);
    ///desctructor
    ///closes the directory
    ~Directory();
    ///returns a vector of the subdirectories
    ///in the current one
    std::vector<Directory> subDirectories();
    ///returns a vector with the paths of the
    ///regular files inside the current directory
    std::vector<std::string> regularFiles();
    ///get the path of the directory
    std::string path()const
    {
        return path_;
    }
};

///exception class that will be
///thrown if a problem appears when
///working with the directory class
class DirectoryException : public std::exception
{
private:
    std::string message_;
public:
    DirectoryException(unsigned int errnoCopy);
    const char* what()const noexcept override
    {
        return message_.c_str();
    }
};

///returns true if the file defined
///by the filePath is a regular file
bool isRegularFile(std::string filePath);

///returns true if path specifies a directory
bool isDirectory(std::string path);
#endif

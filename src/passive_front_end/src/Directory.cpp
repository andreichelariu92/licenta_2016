//standard library headers
//OS headers
#include <errno.h>
//my headers
#include "Directory.h"

using std::vector;
using std::string;

Directory::Directory(std::string path)
    :path_(path),
     dirStructure_(0)
{
    dirStructure_ = opendir(path_.c_str());

    if (dirStructure_ == 0)
    {
        DirectoryException e(errno);
        throw e;
    }
    
    //close the directory, to avoid
    //having too many directories opened
    //at the same time
    closedir(dirStructure_);
    dirStructure_ = 0;
}

Directory::Directory(Directory&& source)
    :path_(source.path_),
     dirStructure_(std::move(source.dirStructure_))
{
    //mark the source as empty, to
    //avoid the source being cleared
    //by its destructor
    source.path_ = "";
    source.dirStructure_ = 0;
}

Directory& Directory::operator=(Directory&& source)
{
    path_ = source.path_;
    dirStructure_ = std::move(source.dirStructure_);
    //mark the source as empty, to
    //avoid the source being cleared
    //by its destructor
    source.path_ = "";
    source.dirStructure_ = 0;
    
    return *this;
}

Directory::~Directory()
{
    if (dirStructure_)
    {
        closedir(dirStructure_);
    }
}

vector<Directory> Directory::subDirectories()
{
    //it the structure was closed
    //try open it
    if (dirStructure_ == 0)
    {
        dirStructure_ = opendir(path_.c_str());
        //throw exception if something went wrong
        if (dirStructure_ == 0)
        {
            DirectoryException e(errno);
            throw e;
        }
    }

    //INSPIRATION:
    //http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
    vector<Directory> subDirs;
    struct dirent* dirEntry;

    //iterate over the entries of the current directory
    while ((dirEntry = readdir(dirStructure_)) != 0)
    {
        //if the current entry is a directory
        if (dirEntry->d_type == DT_DIR ||
            dirEntry->d_type == DT_UNKNOWN)
        {
            string dirName(dirEntry->d_name);
            if (dirName != "." &&
                dirName != "..")
            {
                //create a Directory function and
                //add it to the vector
                string subDirPath = path_ + "/" + dirEntry->d_name;
                //add the subDir only if there
                //is no exception
                try
                {
                    Directory subDir(subDirPath);
                    subDirs.push_back(std::move(subDir));
                }
                catch(DirectoryException& de)
                {
                    //if there is an exception,
                    //catch it, to keep the correct
                    //directories in the vector and
                    //return them
                }
                
            }
        }
    }
    //close the current dirStructure_
    //and set it to 0
    //subsequent calls to this method must open
    //the dirStructure_ again
    closedir(dirStructure_);
    dirStructure_ = 0;
    
    return subDirs;
}

vector<string> Directory::regularFiles()
{
    if (dirStructure_ == 0)
    {
        dirStructure_ = opendir(path_.c_str());

        if (dirStructure_ == 0)
        {
            DirectoryException e(errno);
            throw e;
        }
    }

    //INSPIRATION:
    //http://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
    vector<string> regFiles;
    struct dirent* dirEntry;

    //iterate over the entries of the current directory
    while ((dirEntry = readdir(dirStructure_)) != 0)
    {
        //if the current entry is a regular file
        if (dirEntry->d_type == DT_REG)
        {
            //create a Directory function and
            //add it to the vector
            string filePath = path_ + "/" + dirEntry->d_name;
            regFiles.push_back(filePath);
        }
    }

    //the rationale is the same as above
    closedir(dirStructure_);
    dirStructure_ = 0;
    
    return regFiles;

}
DirectoryException::DirectoryException(unsigned int errnoCopy)
    :message_()
{
    if (errnoCopy == EACCES)
    {
        message_ = "Permision denied";
    }
    else if (errnoCopy == EMFILE || errnoCopy == ENFILE)
    {
        message_ = "Too many directories opened";
    }
    else if (errnoCopy == ENOTDIR || errnoCopy == ENOENT)
    {
        message_ = "not a directory";
    }
    else
    {
        char message[50];
        sprintf(message, "Other type of error %d", errnoCopy);
        message_ = message;
    }
}

bool isRegularFile(std::string filePath)
{
    //INSPIRATION:
    //http://www.cplusplus.com/reference/string/string/find_last_of/
    
    //construct a Directory of the parent
    //of the filePath
    size_t slashIdx = filePath.find_last_of('/');
    if (slashIdx == string::npos)
    {
        return false;
    }
    else
    {
        string parentPath = filePath.substr(0, slashIdx);
        try
        {
            Directory parentDir(parentPath);
            vector<string> files = parentDir.regularFiles();
            for (string file : files)
            {
                if (file == filePath)
                    return true;
            }
            return false;
        }
        catch(...)
        {
            return false;
        }
    }
}

bool isDirectory(std::string path)
{
    //try to create a Directory instance
    //for the given path
    try
    {
        Directory d(path);
        //if there is no exception return
        //true
        return true;
    }
    catch(...)
    {
        //if there is an exception return
        //false
        return false;
    }
}

vector<Directory> getAllDirectories(string path)
{
    vector<Directory> output;
    //create a directory for the given path
    Directory root(path);
    vector<Directory> subDirs = root.subDirectories();
    //for all the subDirectories get their
    //sub directories and add them to the vector,
    //then add the sub directory itself
    for (Directory& subDir : subDirs)
    {
        vector<Directory> subSubDirs =
            getAllDirectories(subDir.path());
        for(Directory& subSubDir : subSubDirs)
        {
            output.push_back(std::move(subSubDir));
        }
    }
    //at the end, add the root
    //directory
    output.push_back(std::move(root));
    
    return output;
}

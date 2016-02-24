#ifndef InotifyDirectory_INCLUDE_GUARD
#define InotifyDirectory_INCLUDE_GUARD

#include <string>
#include <exception>

///Class that uses the inotify API
///to listen for filesystem events
///inside the directory
///The events are not listened recursively!
class InotifyDirectory
{
private:
    int fileDescriptor_;
    int watchDescriptor_;
    std::string path_;
public:
    InotifyDirectory(std::string path, unsigned mask);
    ///copy operations are not permited
    InotifyDirectory(const InotifyDirectory& source)=delete;
    InotifyDirectory& operator=(const InotifyDirectory& source)=delete;
    ///only move operations are permited
    InotifyDirectory(InotifyDirectory&& source);
    InotifyDirectory& operator=(InotifyDirectory&& source);
    ///destructor
    ~InotifyDirectory();
    ///access the internal file descriptor
    ///with this fd, the application can read
    ///the file system modifications
    int fileDescriptor()
    {
        return fileDescriptor_;
    }
    ///access the internal watchDescriptor
    int watchDescriptor()
    {
        return watchDescriptor_;
    }
    ///access the path
    std::string path()
    {
        return path_;
    }
};

///Class that represents an error occured
///when using the inotify API
class InotifyException : public std::exception
{
private:
    std::string message_;
public:
    InotifyException(std::string message)
        :message_(message)
    {}

    const char* what()const noexcept override
    {
        return message_.c_str();
    }
};
#endif

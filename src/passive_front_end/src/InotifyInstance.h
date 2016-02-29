//standard library headers
#include <vector>
#include <string>
//OS headers
#include <sys/inotify.h>
//my headers


class InotifyInstance
{
private:
    int fd_;
    std::vector<int> vectorWd_;
public:
    ///default constructor
    ///throws InotifyException
    ///if there is an error during initialization
    InotifyInstance();
    ///inhibit copy operations
    InotifyInstance(const InotifyInstance& source) = delete;
    InotifyInstance& operator=(const InotifyInstance) = delete;
    ///move operations
    InotifyInstance(InotifyInstance&& source);
    InotifyInstance& operator=(InotifyInstance&& source);
    ///destructor
    ///removes the watch descriptors
    ///and closes the file handle
    ~InotifyInstance();
    ///add the file/directory with the specified
    ///path to the InotifyInstance
    ///mask specifies which events are listened
    ///returns the watch descriptor from inotify API
    int addToWatch(std::string path, unsigned mask);
    ///removes a watch descriptor from the instance
    ///returns true or false if the operation succeeded
    bool removeFromWatch(int wd);
    ///reads from the InotifyInstance the events generated
    ///on the watch descriptors
    ///the function uses poll internally
    //std::vector<struct inotify_event> readEvents(int timeout);
};

class InotifyException : public std::exception
{
private:
    std::string message_;
public:
    InotifyException(int copyOfErrno);
    const char* what() const noexcept override
    {
        return message_.c_str();
    }
};

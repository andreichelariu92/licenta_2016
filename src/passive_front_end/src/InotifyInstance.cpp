//standard library headers
//OS headers
#include <sys/inotify.h>
#include <errno.h>
#include <poll.h>
#include <unistd.h>
#include <limits.h>
//my headers
#include "InotifyInstance.h"

using std::string;
using std::vector;

//INSPIRATION:
//https://www.ibm.com/developerworks/library/l-ubuntu-inotify/

InotifyInstance::InotifyInstance()
    :fd_(0),
     vectorWd_()
{
    fd_ = inotify_init();
    if (fd_ == -1)
    {
        InotifyException e(errno);
        throw e;
    }
}

InotifyInstance::InotifyInstance(InotifyInstance&& source)
    :fd_(source.fd_),
     vectorWd_(source.vectorWd_)
{
    //mark the file descriptor of the source
    //as empty
    source.fd_ = 0;
}

InotifyInstance& InotifyInstance::operator=(InotifyInstance&& source)
{
    fd_ = source.fd_;
    vectorWd_ = source.vectorWd_;
    //mark the file descriptor of the source
    //as empty
    source.fd_ = 0;
    return *this;
}

InotifyInstance::~InotifyInstance()
{
    //remove all the watch descriptors
    for (int wd : vectorWd_)
    {
        inotify_rm_watch(fd_, wd);
    }
    //close the file descriptor
    if (fd_)
        close(fd_);
}

int InotifyInstance::addToWatch(string path, unsigned mask)
{
    int result = inotify_add_watch(fd_, path.c_str(), mask);
    if (result > 0)
    {
        vectorWd_.push_back(result);
    }
    return result;
}

bool InotifyInstance::removeFromWatch(int wd)
{
    int result = inotify_rm_watch(fd_, wd);
    if (result == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

InotifyException::InotifyException(int copyOfErrno)
    :message_()
{
    //TODO: Andrei: Implement :)
}

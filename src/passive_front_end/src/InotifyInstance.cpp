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
bool InotifyInstance::performPoll(int timeout)
{
    //prepare the structure for the poll function
    struct pollfd pollStruct;
    pollStruct.fd = fd_;
    //only interested in reading data
    pollStruct.events = POLLIN;

    int ready = poll(&pollStruct, 1, timeout);
    if (ready)
        return true;
    else
        return false;
}
vector<InotifyEvent> InotifyInstance::readEvents(int timeout)
{
    vector<InotifyEvent> events;
    bool dataReady = performPoll(timeout);
    if (!dataReady)
    {
        //if there is no data ready,
        //return en empty vector
        return events;
    }
    //read 256 events in an auxiliary buffer
    constexpr int EVENT_SIZE = sizeof(struct inotify_event);
    //the buffer must hold 256 inotify_event structures
    //and the name for every file; the name can be
    //of maximum NAME_MAX bytes
    constexpr int BUF_LEN = 256 * (EVENT_SIZE + NAME_MAX);
    char buffer[BUF_LEN];
    int offset = 0;
    
    int length = read(fd_, buffer, BUF_LEN);
    while (offset < length)
    {
        //convert the bytes at the offset
        //to an inotify_event structure
        //and get the addres of that structure
        //in the variable event
        struct inotify_event* event = 
            reinterpret_cast<struct inotify_event*>(&buffer[offset]);
        InotifyEvent iEvent;
        iEvent.wd = event-> wd;
        iEvent.mask = event->mask;
        iEvent.path = string(event->name);
        iEvent.cookie = event->cookie;

        events.push_back(iEvent);

        offset += EVENT_SIZE + event->len;
    }

    return events;
}

InotifyException::InotifyException(int copyOfErrno)
    :message_()
{
    if (copyOfErrno == EMFILE 
        && copyOfErrno == ENFILE
        && copyOfErrno == ENOMEM)
    {
        message_ = "Too many file descriptors";
    }
}

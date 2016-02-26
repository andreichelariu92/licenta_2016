#include "InotifyDirectory.h"

#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>

//INSPIRATION:
//http://www.ibm.com/developerworks/library/l-ubuntu-inotify/

InotifyDirectory::InotifyDirectory(std::string path, unsigned mask)
    :fileDescriptor_(0),
     watchDescriptor_(0),
     path_(path),
     blocking_(true)

{
    //create file descriptor
    fileDescriptor_ = inotify_init();
    if (fileDescriptor_ < 0)
    {
        std::string errorMessage;
        if (errno == EMFILE)
        {
            errorMessage = "limit of inotify instances ";
            errorMessage += "has been reached";
        }
        else if (errno == ENFILE)
        {
            errorMessage = "limit of file descriptors ";
            errorMessage += "has been reached";
        }
        else
        {
            errorMessage = "error on inotify_init()";
        }
        InotifyException e(errorMessage);
        throw e;
    }
    //create write descriptor, only
    //if the file descriptor is correct
    watchDescriptor_ = inotify_add_watch
                       (fileDescriptor_,
                        path_.c_str(),
                        mask);
    if (watchDescriptor_ < 0)
    {
        //close the file descriptor
        //before exiting the function
        //via throwing of exception
        close(fileDescriptor_);
        
        std::string errorMessage;
        if (errno == EBADF)
        {
            errorMessage = "file descriptor is not valid";
        }
        else if (errno == ENOENT)
        {
            errorMessage = "path name is not valid";
        }
        else
        {
            errorMessage = "error on inotify_add_watch()";
        }
        InotifyException e(errorMessage);
        throw e;
    }
}

InotifyDirectory::InotifyDirectory(InotifyDirectory&& source)
    :fileDescriptor_(source.fileDescriptor_),
     watchDescriptor_(source.watchDescriptor_),
     path_(source.path_),
     blocking_(source.blocking_)
{
    //steal the file descriptor and the
    //watch descriptor from the source
    source.fileDescriptor_ = 0;
    source.watchDescriptor_ = 0;
}

InotifyDirectory& InotifyDirectory::operator=
                  (InotifyDirectory&& source)
{
    //close the file descriptor and
    //the watch descriptor, if this has any
    if (fileDescriptor_ && watchDescriptor_)
    {
        inotify_rm_watch(fileDescriptor_, watchDescriptor_);
        close(fileDescriptor_);
    }
    //steal the file descriptor and the
    //watch descriptor from the source
    fileDescriptor_ = source.fileDescriptor_;
    watchDescriptor_ = source.watchDescriptor_;
    blocking_ = source.blocking_;
    source.fileDescriptor_ = 0;
    source.watchDescriptor_ = 0;
    source.blocking_ = true;
    
    return *this;
}

InotifyDirectory::~InotifyDirectory()
{
    if (fileDescriptor_ && watchDescriptor_)
    {
        inotify_rm_watch(fileDescriptor_, watchDescriptor_);
        close(fileDescriptor_);
    }
}

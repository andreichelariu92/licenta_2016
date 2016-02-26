#include "SystemCallsWrappers.h"

#include <poll.h>

using std::vector;
void pollInotifyDirectories(vector<InotifyDirectory>& directories,
                            unsigned flags,
                            int timeout)
{
    //vector that holds the structures passed
    //to the poll sistem call
    std::vector<struct pollfd> pollFileDescriptors;
    pollFileDescriptors.resize(directories.size());
   
    //populate the structures
    const int numFds = directories.size();
    for (int i = 0; i < numFds; ++i)
    {
        pollFileDescriptors[i].fd = directories[i].fileDescriptor();
        pollFileDescriptors[i].events = flags;
    }
    //perform poll system call
    int nrReadyFds = poll(&pollFileDescriptors[0],
                                      pollFileDescriptors.size(),
                                      timeout);
    //mark the ready InotifyDirectories
    //as unblocking
    for (int fdIdx = 0; fdIdx < nrReadyFds; ++fdIdx)
    {
        if (pollFileDescriptors[fdIdx].revents != 0)
        {
            //find the InotifyDirectory with the
            //corresponding fd
            for (unsigned int dirIdx = 0; 
                 dirIdx < directories.size(); 
                 ++dirIdx)
            {
                if (directories[dirIdx].fileDescriptor()
                    ==
                    pollFileDescriptors[fdIdx].fd)
                {
                    directories[dirIdx].unblock();
                }
            }
        }
    }
}

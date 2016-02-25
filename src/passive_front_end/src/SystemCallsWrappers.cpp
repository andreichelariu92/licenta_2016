#include "SystemCallsWrappers.h"

#include <poll.h>

std::vector<int> pollWrapper(std::vector<int> fds,
                             int timeout,
                             unsigned flags)
{
    //vector that holds the structures passed
    //to the poll sistem call
    std::vector<struct pollfd> pollFileDescriptors;
    pollFileDescriptors.resize(fds.size());
   
    //populate the structures
    const int numFds = fds.size();
    for (int i = 0; i < numFds; ++i)
    {
        pollFileDescriptors[i].fd = fds[i];
        pollFileDescriptors[i].events = flags;
    }
    //perform poll system call
    int nrReadyFileDescriptors = poll(&pollFileDescriptors[0],
                                      pollFileDescriptors.size(),
                                      timeout);
    
    //put the ready file descriptors in a vector
    std::vector<int> readyFileDescriptors;
    if (nrReadyFileDescriptors)
    {
        readyFileDescriptors.resize(nrReadyFileDescriptors);
        int j = 0;
        for (int i = 0; i < numFds; ++i)
        {
            if (pollFileDescriptors[i].revents != 0)
            {
                readyFileDescriptors[j] = pollFileDescriptors[i].fd;
                j++;
            }
            //exit the loop when all the ready
            //file descriptors have been added to
            //the vector
            if (j >= nrReadyFileDescriptors)
                break;
        }
    }

    return readyFileDescriptors;
}

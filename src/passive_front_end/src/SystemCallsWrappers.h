#ifndef SystemCallsWrappers_INCLUDE_GUARD
#define SystemCallsWrappers_INCLUDE_GUARD

#include <vector>
#include <poll.h>

///wrapper function around the system call poll
//returns a vector of file descriptors which
//are ready for the operations specified in flags,
//usually read or write
std::vector<int> pollWrapper(std::vector<int> fds, 
                             int timeout,
                             unsigned flags = POLLIN);
#endif

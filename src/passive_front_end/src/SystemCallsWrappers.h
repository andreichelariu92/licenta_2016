#ifndef SystemCallsWrappers_INCLUDE_GUARD
#define SystemCallsWrappers_INCLUDE_GUARD

#include <poll.h>
#include "InotifyDirectory.h"

void pollInotifyDirectories(std::vector<InotifyDirectory>& d,
                            unsigned flags,
                            int timeout);
#endif

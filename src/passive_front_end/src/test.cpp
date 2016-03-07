//standard library headers
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <map>
//OS headers
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <poll.h>
#include <limits.h>
//my headers
#include "Directory.h"
#include "InotifyInstance.h"

using std::vector;
using std::string;
using std::cout;
using std::map;

int main()
{
    //INSPIRATION:
    //http://www.ibm.com/developerworks/library/l-ubuntu-inotify/
    
    //TO DO: Andrei: Test more carefully
    /*
    //data for the inotify API
    //members of the wrapper class
    constexpr int EVENT_SIZE = sizeof(struct inotify_event);
    constexpr int BUFFER_SIZE = 1024 * (EVENT_SIZE + 16);
    char buffer[BUFFER_SIZE];
    int length = 0;
    int i = 0;
    
    InotifyDirectory dir("/home/andrei/test",
                         IN_MODIFY | IN_CREATE | IN_DELETE);
    
    std::vector<InotifyDirectory> dirs; 
    dirs.push_back(std::move(dir));
    pollInotifyDirectories(dirs, POLLIN, 10000);
    if (dirs[0].blocking() == true)
    {
        std::cout << "There have been no events\n";
    }
    else
    {
        //read the data
        length = read(dirs[0].fileDescriptor(),
                      buffer,
                      BUFFER_SIZE);

        if (length < 0)
        {
            perror("read");
        }

        while (i < length)
        {
            struct inotify_event* event = (struct inotify_event*)(&buffer[i]);
            if (event->mask & IN_CREATE)
            {
                std::cout << event->name << " was created\n";
            }
            else if (event->mask & IN_DELETE)
            {
                std::cout << event->name << " was deleted\n";
            }
            else if (event->mask & IN_MODIFY)
            {
                std::cout << event->name << " was modified\n";
            }

            i += EVENT_SIZE + event->len;
        }
    }
    */
    
    vector<Directory> dirs = getAllDirectories("/home/andrei/windows/Andrei/Facultate/anIV");
    map<int, Directory> watchedDirectories;
    InotifyInstance inotify;
    for (Directory& dir : dirs)
    {
        int wd = inotify.addToWatch(dir.path());

        if (wd > 0)
        {
            //add to the watched directories
            //the current one;
            //the directory will be moved from
            //the vector to the map
            watchedDirectories.insert(
                    std::make_pair<int, Directory>(
                        std::move(wd), std::move(dir)));
        }
    }
    constexpr int minute = 60000;
    int nrMinutes = 0;
    while (nrMinutes < 5)
    {
        cout << "Minute " << nrMinutes + 1 << "\n";
        vector<InotifyEvent> events = inotify.readEvents(minute);
        for (InotifyEvent event : events)
        {
            cout << "parent directory: "
                 << watchedDirectories.at(event.wd).path()
                 << " file/dir name: "
                 << event.path
                 << "event type: ";
            if (event.mask & IN_CREATE)
                cout << "CREATE\n";
            else if (event.mask & IN_DELETE)
                cout << "DELETE\n";
            else
                cout << "MODIFY\n";
        }
        ++nrMinutes;
    }
    return 0;
}

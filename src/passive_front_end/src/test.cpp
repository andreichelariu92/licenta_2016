//standard library headers
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
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
    /*
    Directory d("/home/andrei");
    vector<string> files = d.regularFiles();
    for (string file : files)
    {
        std::cout << file << "\n";
    }
    std::this_thread::sleep_for (std::chrono::seconds(10)); 
    vector<Directory> dirs = d.subDirectories();
    for (Directory& dir : dirs)
    {
        std::cout << dir.path() << "\n";
    }

    std::cout << isRegularFile("/home/andrei/main.cpp") << "\n";
    std::cout << isDirectory("/home/andrei") << "\n";
    */
    InotifyInstance ii;
    Directory d("/home/andrei/windows");
    vector<Directory> subDirs = d.subDirectories();
    vector<int> wds;
    for (Directory& subDir : subDirs)
    {
        //cout << subDir.path() << "\n";
        int wd = ii.addToWatch(subDir.path(),
                      IN_CREATE | IN_DELETE | IN_MODIFY);
        wds.push_back(wd);
    }
    vector<InotifyEvent> events = ii.readEvents(10000);
    for (InotifyEvent& event : events)
    {
        cout << event.path;
        if (event.mask & IN_DELETE)
        {
            cout << " was deleted\n";
        }
        else if (event.mask & IN_CREATE)
        {
            cout << " was created\n";
        }
        else if (event.mask & IN_MODIFY)
        {
            cout << " was modified\n";
        }
    }
    return 0;
}

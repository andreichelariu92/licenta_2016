#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>

int main()
{
    //INSPIRATION:
    //http://www.ibm.com/developerworks/library/l-ubuntu-inotify/

    //data for the inotify API
    //members of the wrapper class
    constexpr int EVENT_SIZE = sizeof(struct inotify_event);
    constexpr int BUFFER_SIZE = 1024 * (EVENT_SIZE + 16);
    char buffer[BUFFER_SIZE];
    int fileDescriptor;
    int writeDescriptor;
    int length = 0;
    int i = 0;
    
    //initialize the api
    //this code will be in the constructor of the wrapper
    fileDescriptor = inotify_init();
    if (fileDescriptor < 0)
    {
        perror("inotify_init");
    }

    writeDescriptor = inotify_add_watch(
                        fileDescriptor,
                        "/home/andrei/test",
                        IN_MODIFY | IN_CREATE | IN_DELETE);
    //read the data
    //this code will be in the read method
    length = read(fileDescriptor,
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

    inotify_rm_watch(fileDescriptor, writeDescriptor);
    close(fileDescriptor);
    return 0;
}

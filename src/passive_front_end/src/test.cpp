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
#include "DirectoryWatcher.h"

using std::vector;
using std::string;
using std::cout;
using std::map;

int main()
{
    //test results
    //63.7 MB consumed -> 590.5 MB file hierarchy
    //215.1 MB consumed -> 13.263 GB file hierarchy
    //
    //test hierarchy:
    //test
    //  /dir1
    //      file1
    //      /dir2
    //          file2
    DirectoryWatcher dw("/home/andrei/test");
    constexpr int minute = 60000;
    for (unsigned int minuteIdx = 0;
         minuteIdx < 5;
         ++minuteIdx)
    {
        cout << "Minute: " << minuteIdx + 1 << "\n";
        vector<FileEvent> fEvents = dw.readEvents(minute);

        for (FileEvent& fEvent : fEvents)
        {
            cout << fEvent.absolutePath << " ";
            switch (fEvent.fileType)
            {
                case FileType::file:
                    cout << "file ";
                    break;
                case FileType::directory:
                    cout << "dir ";
                    break;
            }
            switch (fEvent.eventType)
            {
                case EventType::create:
                    cout << "create ";
                    break;
                case EventType::modified:
                    cout << "modified ";
                    break;
                case EventType::deleted:
                    cout <<"deleted ";
                    break;
                case EventType::movedFrom:
                    cout << "moved from ";
                    break;
                case EventType::movedTo:
                    cout << "moved to ";
                    break;
            }
            cout << "\n";
        }
    }
    return 0;
}

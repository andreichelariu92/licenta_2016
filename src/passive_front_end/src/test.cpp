//standard library headers
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <map>
#include <fstream>
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
using std::cerr;
using std::map;
using std::ofstream;

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
         minuteIdx < 200;
         ++minuteIdx)
    {
        cerr << "Minute: " << minuteIdx + 1 << "\n";
        vector<FileEvent> fEvents = dw.readEvents(minute);

        for (FileEvent& fEvent : fEvents)
        {
            cerr << fEvent.absolutePath << " ";
            switch (fEvent.fileType)
            {
                case FileType::file:
                    cerr << "file ";
                    break;
                case FileType::directory:
                    cerr << "dir ";
                    break;
            }
            switch (fEvent.eventType)
            {
                case EventType::create:
                    cerr << "create ";
                    break;
                case EventType::modified:
                    cerr << "modified ";
                    break;
                case EventType::deleted:
                    cerr <<"deleted ";
                    break;
                case EventType::movedFrom:
                    cerr << "moved from ";
                    break;
                case EventType::movedTo:
                    cerr << "moved to ";
                    break;
                default:
                    break;
            }
            cerr << "\n";
        }
    }
    return 0;
}

//standard library headers
#include <utility>
#include <iostream>
//OS headers
//my headers
#include "DirectoryWatcher.h"

using std::vector;
using std::pair;
using std::string;
using std::cout;

DirectoryWatcher::DirectoryWatcher(string rootDir)
    :watchedDirectories_(),
     inotify_()
{
    registerDirectories(rootDir);
}

void DirectoryWatcher::registerDirectories(string rootDir)
{
    //get all the directories in the file hierarchy
    //starting at rootDir
    vector<Directory> dirs = getAllDirectories(rootDir);
    //try to add the directories
    //to the Inotify instance
    for (Directory& dir : dirs)
    {
        int wd = inotify_.addToWatch(dir.path());
        if (wd > 0)
        {
            pair<int, Directory> p(std::move(wd), std::move(dir));
            watchedDirectories_.insert(std::move(p));
        }
    }
}

DirectoryWatcher::DirectoryWatcher(DirectoryWatcher&& source)
    : watchedDirectories_(std::move(source.watchedDirectories_)),
      inotify_(std::move(source.inotify_))
{
}

DirectoryWatcher& DirectoryWatcher::operator=(
        DirectoryWatcher&& source)
{
    watchedDirectories_ = std::move(source.watchedDirectories_);
    inotify_ = std::move(source.inotify_);
    return *this;
}

vector<FileEvent> DirectoryWatcher::readEvents(int timeout)
{
    vector<FileEvent> fEvents;

    vector<InotifyEvent> iEvents =
        inotify_.readEvents(timeout);
    
    for (InotifyEvent& iEvent : iEvents)
    {
        //get a reference to the coresponding
        //directory
        Directory& parentDir = watchedDirectories_.at(iEvent.wd);
        //fill in a FileEvent structure
        //and put it in the return vector
        string absolutePath = parentDir.path() + "/" + iEvent.path;

        FileType fileType;
        if (iEvent.mask & IN_ISDIR)
        {
            fileType = FileType::directory;
        }
        else
        {
            fileType = FileType::file;
        }

        EventType eventType;
        if (iEvent.mask & IN_CREATE)
        {
            if (fileType == FileType::directory)
            {
                //add the directory and its contents
                //to watchedDirectories_
                registerDirectories(absolutePath);
            }
            eventType = EventType::create;
        }
        else if (iEvent.mask & IN_MODIFY)
        {
            eventType = EventType::modified;
        }
        else if (iEvent.mask & IN_DELETE_SELF ||
                 iEvent.mask & IN_DELETE)
        {
            eventType = EventType::deleted;
        }
        FileEvent fEvent(absolutePath, fileType, eventType);
        fEvents.push_back(fEvent);
    }
    
    return fEvents;

}

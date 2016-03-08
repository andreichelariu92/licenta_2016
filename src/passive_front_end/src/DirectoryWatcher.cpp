//standard library headers
#include <utility>
//OS headers
//my headers
#include "DirectoryWatcher.h"

using std::vector;
using std::pair;
using std::string;

DirectoryWatcher::DirectoryWatcher(string rootDir)
    :watchedDirectories_(),
     inotify_()
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
        if (isDirectory(absolutePath))
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
                //TODO : Andrei
                //add the directory and its contents
                //to watchedDirectories_
            }
            eventType = EventType::create;
        }
        else if (iEvent.mask & IN_MODIFY)
        {
            eventType = EventType::modified;
        }
        else
        {
            if (fileType == FileType::directory)
            {
                //TODO: Andrei
                //check if the wd are removed automatically
                //if not, remove them from inotify instance
            }
            eventType = EventType::deleted;
        }

        FileEvent fEvent(absolutePath, fileType, eventType);
        fEvents.push_back(fEvent);
    }
    
    return fEvents;

}

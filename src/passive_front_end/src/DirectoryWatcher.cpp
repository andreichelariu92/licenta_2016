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
     directoryNames_(),
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
            directoryNames_.insert(string(dir.path()));
            pair<int, Directory> p(std::move(wd), std::move(dir));
            watchedDirectories_.insert(std::move(p));
        }
    }
}

DirectoryWatcher::DirectoryWatcher(DirectoryWatcher&& source)
    : watchedDirectories_(std::move(source.watchedDirectories_)),
      directoryNames_(std::move(source.directoryNames_)),
      inotify_(std::move(source.inotify_))
{
}

DirectoryWatcher& DirectoryWatcher::operator=(
        DirectoryWatcher&& source)
{
    watchedDirectories_ = std::move(source.watchedDirectories_);
    directoryNames_ = std::move(source.directoryNames_);
    inotify_ = std::move(source.inotify_);
    return *this;
}

EventType DirectoryWatcher::getEventType(const InotifyEvent& iEvent)
{
    EventType eventType = EventType::invalid;
    if (iEvent.mask & IN_CREATE)
    {
        eventType = EventType::create;
    }
    else if (iEvent.mask & IN_MODIFY)
    {
        eventType = EventType::modified;
    }
    else if (iEvent.mask & IN_DELETE)
    {
        eventType = EventType::deleted;
    }
    else if (iEvent.mask & IN_MOVED_FROM)
    {
        eventType = EventType::movedFrom;
    }
    else if (iEvent.mask & IN_MOVED_TO)
    {
        eventType = EventType::movedTo;
    }

    return eventType;
}

vector<FileEvent> DirectoryWatcher::readEvents(int timeout)
{
    //TODO: Andrei:
    //1)Check for file move
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
        
        //flag used to remove the dir
        //from the map watchedDirectories_
        bool selfRemove = false;

        //get the event type
        EventType eventType = getEventType(iEvent);
        if (iEvent.mask & IN_DELETE_SELF)
        {
            selfRemove = true;
        }
        
        //set the file type
        FileType fileType;
        if (eventType == EventType::create)
        {
            //use IN_ISDIR only in the case the file
            //was created
            //in the other cases use the map, beacuse
            //it is more reliable
            if (iEvent.mask & IN_ISDIR)
            {
                fileType = FileType::directory;
                //register all the subdirectories
                //of the newly created one
                registerDirectories(absolutePath);
                //add the name to the set
                directoryNames_.insert(absolutePath);
            }
            else
            {
                fileType = FileType::file;
            }
        }
        else if (eventType == EventType::modified)
        {
            if (directoryNames_.find(absolutePath)
                    != directoryNames_.end())
            {
                fileType = FileType::directory;
            }
            else
            {
                fileType = FileType::file;
            }
        }
        else if (eventType == EventType::deleted)
        {
            if (directoryNames_.find(absolutePath)
                    != directoryNames_.end())
            {
                fileType = FileType::directory;
                //remove its name from the set
                directoryNames_.erase(absolutePath);
            }
            else
            {
                fileType = FileType::file;
            }
        }
        else if (eventType == EventType::movedFrom)
        {

        }
        else if (eventType == EventType::movedTo)
        {

        }
        else if (selfRemove &&
            (directoryNames_.find(absolutePath) != 
             directoryNames_.end())
           )
        {
            //if there was a self remove event for a
            //directory, remove it from the map
            //do not generate file event for it.
            //it will be generated when the delete event
            //is received by the parent directory of the
            //deleted one
            watchedDirectories_.erase(iEvent.wd);
        }
        if (eventType != EventType::invalid)
        {
            FileEvent fEvent(absolutePath, fileType, eventType);
            fEvents.push_back(fEvent);
        }
    }
    
    return fEvents;

}

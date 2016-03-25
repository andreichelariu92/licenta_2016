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
using std::map;

DirectoryWatcher::DirectoryWatcher(string rootDir)
    :watchedDirectories_(),
     directoryNames_(),
     inotify_(),
     movedDirectories_()
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
      inotify_(std::move(source.inotify_)),
      movedDirectories_(std::move(source.movedDirectories_))
{
}

DirectoryWatcher& DirectoryWatcher::operator=(
        DirectoryWatcher&& source)
{
    watchedDirectories_ = std::move(source.watchedDirectories_);
    directoryNames_ = std::move(source.directoryNames_);
    inotify_ = std::move(source.inotify_);
    movedDirectories_ = std::move(source.movedDirectories_);
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
FileType DirectoryWatcher::getFileType(const InotifyEvent& iEvent,
        const string& absolutePath, const EventType& eventType)
{
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
        }
        else
        {
            fileType = FileType::file;
        }
    }
    else
    {
        if (directoryNames_.find(absolutePath) !=
                directoryNames_.end())
        {
            fileType = FileType::directory;
        }
        else
        {
            fileType = FileType::file;
        }
    }

    return fileType;
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
        string absolutePath = parentDir.path() + "/" + iEvent.path;
        
        //get the event type
        EventType eventType = getEventType(iEvent);
        //get the file type
        FileType fileType = getFileType(iEvent,
                absolutePath, eventType);

        if (eventType == EventType::create &&
                fileType == FileType::directory)
        {
            //register all the subdirectories
            //of the newly created one
            registerDirectories(absolutePath);
            //add the name to the set
            directoryNames_.insert(absolutePath);
        }

        if (eventType == EventType::deleted &&
                fileType == FileType::directory)
        {
            //remove its name from the set
            directoryNames_.erase(absolutePath);
        }

        if (eventType == EventType::movedFrom &&
                fileType == FileType::directory)
        {
            //map the cookie with the source path
            //of the moved directory
            const unsigned int moveCookie = iEvent.cookie;
            pair<int, string> p = 
                std::make_pair(moveCookie, absolutePath);
            movedDirectories_.insert(p);
        }

        if (eventType == EventType::movedTo)
        {
           map<int, std::string>::iterator position = 
               movedDirectories_.find(iEvent.cookie);

           if (position != movedDirectories_.end())
           {
               string source = (*position).second;
               adjustDirectoryPaths(source, absolutePath);
           }
           else
           {
               eventType = EventType::invalid;
           }
        }

        if (iEvent.mask & IN_DELETE_SELF
                && fileType == FileType::directory)
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

void DirectoryWatcher::adjustDirectoryPaths(const string& source,
        const string& destination)
{
    for (pair<const int, Directory>& p : watchedDirectories_)
    {
        if (p.second.isIncluded(source))
        {
            p.second.path(destination);
        }
    }
}

//standard library headers
#include <utility>
#include <iostream>
//OS headers
//my headers
#include "DirectoryWatcher.h"
#include "../../util/src/Logger.h"

using std::vector;
using std::pair;
using std::string;
using std::cerr;
using std::map;

DirectoryWatcher::DirectoryWatcher(string rootDir)
    :watchedDirectories_(),
     inotify_(),
     movedFiles_()
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
        else
        {
           LOG << INFO << Logger::error
               << " " << dir.path()
               << " could not be registered\n";
        }
    }
}

DirectoryWatcher::DirectoryWatcher(DirectoryWatcher&& source)
    : watchedDirectories_(std::move(source.watchedDirectories_)),
      inotify_(std::move(source.inotify_)),
      movedFiles_(std::move(source.movedFiles_))
{
}

DirectoryWatcher& DirectoryWatcher::operator=(
        DirectoryWatcher&& source)
{
    watchedDirectories_ = std::move(source.watchedDirectories_);
    inotify_ = std::move(source.inotify_);
    movedFiles_ = std::move(source.movedFiles_);
    return *this;
}

EventType DirectoryWatcher::getEventType(const InotifyEvent& iEvent)
{
    EventType eventType = EventType::invalid;
    if (iEvent.mask & IN_CREATE)
    {
        eventType = EventType::create;
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
    else if (iEvent.mask & IN_OPEN)
    {
        eventType = EventType::open;
    }
    //only take into account when the file
    //was open with writing permissions
    else if (iEvent.mask & IN_CLOSE_WRITE)
    {
        eventType = EventType::close;
    }


    return eventType;
}
FileType DirectoryWatcher::getFileType(const InotifyEvent& iEvent)
{
    FileType fileType;
    
    if (iEvent.mask & IN_ISDIR)
        fileType = FileType::directory;
    else
        fileType = FileType::file;
    return fileType;
}
vector<FileEvent> DirectoryWatcher::readEvents(int timeout)
{
    vector<FileEvent> fEvents;

    vector<InotifyEvent> iEvents =
        inotify_.readEvents(timeout);
    
    for (InotifyEvent& iEvent : iEvents)
    {
        //ignore the current inotify event if
        //it has no directory associated in the map
        //of watched directories
        //this can happen when a directory was deleted
        //but events for it are still generated
        map<int, Directory>::iterator parentDirPosition;
        parentDirPosition = watchedDirectories_.find(iEvent.wd);
        if (parentDirPosition == watchedDirectories_.end())
        {
            LOG << INFO << Logger::error
                << iEvent.path << " " << iEvent.wd
                << " has been removed from the watched dirs\n";
            continue;
        }

        //get a reference to the coresponding directory
        Directory& parentDir = (*parentDirPosition).second;
        string absolutePath = parentDir.path() + "/" + iEvent.path;
        
        //get the event type
        EventType eventType = getEventType(iEvent);
        //get the file type
        FileType fileType = getFileType(iEvent);

        if (eventType == EventType::create &&
                fileType == FileType::directory)
        {
            //register all the subdirectories
            //of the newly created one
            registerDirectories(absolutePath);
        }

        if (eventType == EventType::movedFrom)
        {
            //map the cookie with the source path
            //of the moved file
            const unsigned int moveCookie = iEvent.cookie;
            pair<int, string> p = 
                std::make_pair(moveCookie, absolutePath);
            movedFiles_.insert(p);
        }

        if (eventType == EventType::movedTo)
        {
           map<int, std::string>::iterator position = 
               movedFiles_.find(iEvent.cookie);

           if (position != movedFiles_.end())
           {
               string source = (*position).second;
               if (fileType == FileType::directory)
               {
                   adjustDirectoryPaths(source, absolutePath);
               }
           }
           else
           {
               eventType = EventType::invalid;
           }
        }
        
        //if there was a self remove event for a
        //directory, remove it from the map
        //do not generate file event for it.
        //it will be generated when the delete event
        //is received by the parent directory of the
        //deleted one
        if (iEvent.mask & IN_DELETE_SELF)
        {
            LOG << INFO << Logger::debug 
                << " delete self "
                << absolutePath
                <<"\n";
            watchedDirectories_.erase(iEvent.wd);
        }
        
        //only take into account the open and close
        //operations on files
        if (eventType == EventType::open && 
            fileType == FileType::directory)
        {
            eventType = EventType::invalid;
        }
        if (eventType == EventType::close &&
            fileType == FileType::directory)
        {
            eventType = EventType::invalid;
        }


        if (eventType != EventType::invalid)
        {
            FileEvent fEvent(absolutePath, fileType, eventType);
            fEvents.push_back(fEvent);
        }
        else
        {
            LOG << INFO << Logger::debug
                << " event on "
                << absolutePath
                << " is invalid\n";
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

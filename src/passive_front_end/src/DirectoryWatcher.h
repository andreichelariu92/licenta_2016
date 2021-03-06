#ifndef DirectoryWatcher_INCLUDE_GUARD
#define DirectoryWatcher_INCLUDE_GUARD

//standard library headers
#include <map>
#include <string>
#include <vector>
#include <set>
//my headers
#include "Directory.h"
#include "InotifyInstance.h"

enum class FileType
{
    file = 0,
    directory = 1
};

enum class EventType
{
    invalid = -1,
    create = 0,
    deleted = 1,
    movedFrom = 2,
    movedTo = 3,
    open = 4,
    closeWrite = 5,
    closeNoWrite = 6
};

struct FileEvent
{
    std::string absolutePath;
    FileType fileType;
    EventType eventType;

    FileEvent(std::string absPath,
              FileType ft,
              EventType et)
        :absolutePath(absPath),
         fileType(ft),
         eventType(et)
    {}
};

///class that watches the files
///and directories inside a file
///hierarchy
class DirectoryWatcher
{
private:
    std::map<int, Directory> watchedDirectories_;
    InotifyInstance inotify_;
    std::map<int, std::string> movedFiles_;
    void registerDirectories(std::string rootDir);
    EventType getEventType(const InotifyEvent& iEvent);
    FileType getFileType(const InotifyEvent& iEvent);
    void adjustDirectoryPaths(const std::string& source,
            const std::string& destination);

public:
    ///constructor
    DirectoryWatcher(std::string rootDir);
    ///inhibit copy operations
    DirectoryWatcher(const DirectoryWatcher& source) = delete;
    DirectoryWatcher& operator=(
            const DirectoryWatcher& source) = delete;
    ///move operations
    DirectoryWatcher(DirectoryWatcher&& source);
    DirectoryWatcher& operator=(DirectoryWatcher&& source);
    ///use the default destructor;
    ///it will call the destructors for Directory
    ///and InotifyInstance classes
    ~DirectoryWatcher() = default;
    ///get the list of events
    ///that occured in the specified interval
    ///(milliseconds)
    std::vector<FileEvent> readEvents(int timeout);
};
#endif

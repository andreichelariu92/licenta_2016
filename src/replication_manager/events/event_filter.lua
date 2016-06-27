local fileops = require("file_operations")
--Table that will hold all the functions available to
--the rest of the application.
local event_filter = {}
------------------------------------------------------------
----------------Local variables-----------------------------
------------------------------------------------------------

--Module variable that holds a list with all the
--ignored file name prefixes. Any file or directory
--that starts with this name, will have its events
--ignored(i.e. not forwarded on the network).
local ignoredPrefixes = {}
--Module variable that holds a list with all the ignored
--paths. All the events that will happpen on these paths
--will be ignored(i.e. not forwarded on the network)
--untill the path is removed from the list
local ignoredPaths = {}
--Module variable that holds a list with all the ignored
--events
local ignoredEvents = {}
------------------------------------------------------------
----------------Local functions-----------------------------
------------------------------------------------------------

------------------------------------------------------------
----------------Public functions----------------------------
------------------------------------------------------------

--Function that initializes the internal state of the module
--It must be called before working with the module
function event_filter.init()
end

--Function that adds a prefix to the list of ignored prefixes
function event_filter.addIgnorePrefix(prefix)
    ignoredPrefixes[#ignoredPrefixes + 1] = prefix
end

--Function that checks if the file at the given path should be
--ignored or not.
function event_filter.isFileIgnored(absolutePath)
    if ignoredPaths[absolutePath] then
        return true
    end

    for _, prefix in pairs(ignoredPrefixes) do
        if fileops.hasPrefix(absolutePath, prefix) then
            return true
        end
    end
    
    return false
end

--Function that checks if the given local event is in the
--list of ignored events. If it is, it removes the event from
--the list and returns true.
function event_filter.isEventIgnored(event)
    for idx, ignoredEvent in pairs(ignoredEvents) do
        if event.absolutePath == ignoredEvent.absolutePath and
           event.fileType == ignoredEvent.fileType and
           event.eventType == ignoredEvent.eventType then
           
            table.remove(ignoredEvents, idx)
            return true
        end
    end

    return false
end

--Function that adds an event to the list of ignored events.
function event_filter.addIgnoreEvent(event)
    table.insert(ignoredEvents, event)
end

--Function that adds a path to the list of ignored paths
function event_filter.addIgnorePath(path)
    ignoredPaths[path] = true
end

--Function that removes a path from the list of ignored paths
function event_filter.removeIgnorePath(path)
    ignoredPaths[path] = nil
end

return event_filter

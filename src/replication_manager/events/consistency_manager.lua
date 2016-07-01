--Local table that will hold the functions exported
--to the rest of the application.
local con_mgr = {}

--Module variable that holds all the opened files.
--When a file is closed, it will be removed from the
--table.
local openFiles = {}

--Module variable that holds all the files in conflict.
--More specifically, all the files that are opened by the
--user and that receive a close event from the network.
local conflictFiles = {}

--Module variable that holds all the conflict events for
--a given path
local conflictEvents = {}

--Function that will initialize all the module variables
--needed by the consistency manager
function con_mgr.init()
end

--Function that handles the open events received from
--the passive_front_end. It stores the opened files
--in an internal table of the module.
function con_mgr.handleLocalOpenEvent(event)
    openFiles[event.absolutePath] = true
end

--Function that handles the close events received
--from the network. Returns true if the file is
--in conflict, i.e. the user has opened the file,
--but not yet closed it.
function con_mgr.handleNetworkEvent(absPath)
    if openFiles[absPath] then
        conflictFiles[absPath] = true
    end

    return conflictFiles[absPath]
end

--Function that handles the close events received
--from the passive_front_end
function con_mgr.handleLocalCloseEvent(event)
    --mark the file as closed
    openFiles[event.absolutePath] = nil

    if conflictFiles[event.absolutePath] then
        conflictFiles[event.absolutePath] = nil
        return true
    else
        return false
    end
end

--Function that adds a conflict event to the given path
function con_mgr.addConflictEvent(path, event)
    if not conflictEvents[path] then
        --create a new list of events
        conflictEvents[path] = {}
    end
    
    local nrEvents = #(conflictEvents[path])
    conflictEvents[path][nrEvents + 1] = event
end

--Function that returns all the conflict events for a given
--path.
function con_mgr.getConflictEvents(path)
    local output = conflictEvents[path]
    --remove the pointer from the module variable
    conflictEvents[path] = nil

    return output
end

return con_mgr

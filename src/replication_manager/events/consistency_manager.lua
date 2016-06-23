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
function con_mgr.handleNetworkCloseEvent(absPath)
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

return con_mgr

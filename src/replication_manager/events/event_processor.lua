local json = require("json")
local fileops = require("file_operations")
local conmgr = require("events.consistency_manager")
local event_filter = require("events.event_filter")

--global table that will hold all the functions visible 
--to the application
event_processor = {}

--Module variable that holds the root directory
local rootDir = ""
--Module variable that holds the prefix of the tentative
--files. Tentative files are files modified on the network
--but that are in the same time modified by the user.
local tentativePrefix = ""
--Module variable that holds the prefix for the conflict
--files. Conflict files are files that contain the modifications
--done by the user, while the file was modified on the network.
local conflictPrefix = ""
--Module variable that holds the prefix for the lock files.
local lockPrefix = ""
--Module variable that holds the function used to solve the
--conflict for a file. It can be set when the module is
--initialized.  localPath is the path to the data modified
--by the user. tentativePath is the path to the file modified on
--the network.
local conflictFunction = function (localPath, tentativePath)
    --The default function moves the file modifed by the
    --user in a file with the conflictPrefix at the begining.
    --It then moves the tentative file(with the data from
    --the network) in place of the user file.
    local conflictPath = fileops.addPrefix(localPath, conflictPrefix)
    
    local moveEvent = {}
    moveEvent.absolutePath = localPath
    moveEvent.fileType = "file"
    moveEvent.eventType = "movedFrom"
    event_filter.ignoreEvent(moveEvent)

    fileops.move(localPath, conflictPath)
    fileops.move(tentativePath, localPath)
end
--Function that initializes the internal state of the module
function event_processor.init(argRootDir, 
                              argLockPrefix, 
                              argTentativePrefix,
                              argConflictPrefix,
                              argConflictFunction)
    rootDir = argRootDir
    
    event_filter.addIgnorePrefix(".")
    
    if not argLockPrefix then
        argLockPrefix = "_lock_"
    end
    lockPrefix = argLockPrefix
    fileops.init(lockPrefix)
    event_filter.addIgnorePrefix(argLockPrefix)

    if argTentativePrefix then
        tentativePrefix = argTentativePrefix
    else
        tentativePrefix = "_tentative_"
    end
    --events on the tentative files will not be forwarded on
    --the network
    event_filter.addIgnorePrefix(tentativePrefix)

    if argConflictPrefix then
        conflictPrefix = argConflictPrefix
    else
        conflictPrefix = "_conflict_"
    end
    --events on the conflict files will not be forwarded
    --on the network
    event_filter.addIgnorePrefix(conflictPrefix)

    if argConflictFunction and 
       type(argConflictFunction) == "function" then
       
       conflictFunction = argConflictFunction
   end

end

--Local function that constructs a network event from the
--given local event
local function makeNetworkEvent(localEvent)
    local output = {}
    output.relativePath = 
        fileops.relativePath(localEvent.absolutePath, rootDir)
    output.eventType = localEvent.eventType
    output.fileType = localEvent.fileType
    
    --add the content of the file to the event only if the
    --file has been modified or created
    if (output.fileType == "file") and (output.eventType == "closeWrite") then
       local file = assert(io.open(localEvent.absolutePath, "rb"))
       output.buffer = file:read("*all")
       file:close()
   else
       output.buffer = ""
   end

   return output
end

--Local function that adds the path of the locked file
--to the list of ignored paths. Its argument is the path
--of the lock file(containing the lock extension at the
--begining of the file name
local function ignoreLockedFile(path)
    if not fileops.isLockFile(path) then
        --do nothing
        return
    end

    local filePath = fileops.removePrefix(path, lockPrefix)
    if filePath then
        event_filter.addIgnorePath(filePath)
    end
end

--Local function that removes the path of the locked file
--from the list of ignored paths. Its argument is the path
--of the lock file
local function deignoreLockedFile(path)
    if not fileops.isLockFile(path) then
        --do nothing
        return
    end

    local filePath = fileops.removePrefix(path, lockPrefix)
    if filePath then
        event_filter.removeIgnorePath(filePath)
    end
end

--Local function that will execute the local event.
--It makes backups(if I have time to implement) and
--returns true if the event should be sent on the network.
local function execLocalEvent(event)
    print("event_processor::execLocalEvent")
   
    if fileops.isLockFile(event.absolutePath) then
        if event.eventType == "create" then
            ignoreLockedFile(event.absolutePath)
        elseif event.eventType == "deleted" then
            deignoreLockedFile(event.absolutePath)
        end
    end

    if event_filter.isFileIgnored(event.absolutePath) then
        return false
    end
    
    if event_filter.isEventIgnored(event) then
        return false
    end

    --don't forward open events
    if event.eventType == "open" then
        --TODO: Andrei: remove
        print(event.absolutePath, "open")
        conmgr.handleLocalOpenEvent(event)
        return false
    end
    
    if event.eventType == "closeWrite" or 
       eventType == "closeNoWrite" then
        local conflict = conmgr.handleLocalCloseEvent(event)
        if conflict then
            print("local event conflict on file ", event.absolutePath)
            local tentativePath = 
                fileops.addPrefix(event.absolutePath, tentativePrefix)
            conflictFunction(event.absolutePath, tentativePath)
        end
    end

    --don't forward closeNoWrite events
    if event.eventType == "closeNoWrite" then
        --TODO: Andrei: remove
        print(event.absolutePath, "closeNoWrite")
        return false
    end

    return true
end

--Local function that sends the network event using
--the broadcast mechanism
local function sendNetworkEvent(networkEvent)
   local eventJson = json.encode(networkEvent)
   print("event_processor::sendNetworkEvent ", eventJson)
   local message = {id = "message1", buffer = eventJson}
   bcast_client.send(message)
end

--Function that processes the events received from the 
--passive front end
--The structure of the events received from pfe is:
--absolutePath = path to the file or directory
--eventType = type of event (create, deleted, open, closeWrite,
--            closeNoWrite, movedTo, movedFrom)
--fileType =  type of file(file, directory)
function event_processor.processLocalEvent(event)
   local send = execLocalEvent(event)
   if send then
       local networkEvent = makeNetworkEvent(event)
       sendNetworkEvent(networkEvent)
   end
end

--Last moved from file
local movedFromFile = ""
--Last moved to dir
local movedFromDir = ""

--Function that processes the events received from the network
--The structure of a network event is:
--relativePath = path of the file relative to the root dir
--eventType = type of event(create, deleted, movedTo, movedFrom,
--            closeWrite)
--fileType = type of file(file, directory)
--buffer = the content of the file
function event_processor.processNetworkEvent(event)
    local absPath = fileops.absolutePath(event.relativePath, rootDir)
    if event.eventType == "create" then
        if event.fileType == "file" then
            fileops.lockFile(absPath)
            fileops.createFile(absPath)
            fileops.unlockFile(absPath)
        elseif event.fileType == "directory" then
            fileops.lockDir(absPath)
            fileops.createDir(absPath)
            fileops.unlockDir(absPath)
        end
    elseif event.eventType == "deleted" then
        if event.fileType == "file" then
            fileops.lockFile(absPath)
            fileops.removeFile(absPath)
            fileops.unlockFile(absPath)
        elseif event.fileType == "directory" then
            fileops.lockDir(absPath)
            fileops.removeDir(absPath)
            fileops.unlockDir(absPath)
        end
    elseif event.eventType == "movedFrom" then
        if event.fileType == "file" then
            fileops.lockFile(absPath)
            movedFromFile = absPath
        elseif event.fileType == "directory" then
            fileops.lockDir(absPath)
            movedFromDir = absPath
        end
    elseif event.eventType == "movedTo" then
        if event.fileType == "file" then
            fileops.move(movedFromFile, absPath)
            fileops.unlockFile(movedFromFile)
            movedFromFile = ""
        elseif event.fileType == "directory" then
            fileops.move(movedFromDir, absPath)
            fileops.unlockDir(movedFromDir)
            movedFromDir = ""
        end
    elseif event.eventType == "closeWrite" then
        local conflict = conmgr.handleNetworkCloseEvent(absPath)
        if conflict then
            print("network event conflict on file ", absPath)
            absPath = fileops.addPrefix(absPath, tentativePrefix)
        end
        fileops.lockFile(absPath)
        fileops.writeFile(absPath, event.buffer)
        fileops.unlockFile(absPath)
    end
end

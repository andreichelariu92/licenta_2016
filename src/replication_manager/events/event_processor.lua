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
--Public variable that holds the prefix for the conflict
--files. Conflict files are files that contain the modifications
--done by the user, while the file was modified on the network.
event_processor.conflictPrefix = ""
--Public variable that holds the prefix for the lock files.
event_processor.lockPrefix = ""
--Module variable that holds the function used to solve the
--conflict for a file. It can be set when the module is
--initialized.  localPath is the path to the data modified
--by the user. conflictEvents holds a list with the network
--event that happend while the file was open.
local _conflictFunction = function (localPath, conflictEvents)
    --The default function makes a copy of the user modifications
    --in a conflict file and then executes the conflict events on
    --the original file.
    local conflictPath = fileops.addPrefix(localPath, 
                           event_processor.conflictPrefix)
    fileops.copyFile(localPath, conflictPath)
    
    --execute the events on the original file
    for _, event in pairs(conflictEvents) do
        logger.log("Conflict network event " .. event.relativePath)
        event_processor.processNetworkEvent(event)
    end
end
--Function that initializes the internal state of the module
function event_processor.init(argRootDir,
                              argLockPrefix, 
                              argTentativePrefix,
                              argConflictPrefix)
    rootDir = argRootDir
    
    event_filter.init()
    event_filter.addIgnorePrefix(".")
    
    if not argLockPrefix then
        argLockPrefix = "_lock_"
    end
    event_processor.lockPrefix = argLockPrefix
    fileops.init(event_processor.lockPrefix)
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
        event_processor.conflictPrefix = argConflictPrefix
    else
        event_processor.conflictPrefix = "_conflict_"
    end
    --events on the conflict files will not be forwarded
    --on the network
    event_filter.addIgnorePrefix(event_processor.conflictPrefix)


end

--Public function that sets the conflict function
function event_processor.setConflictFunction(f)
    if f and type(f) == "function" then
        print("Custom function for conflicts will be used")
       _conflictFunction = f
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
    if (output.fileType == "file") 
       and (output.eventType == "closeWrite") then
       
       local file = io.open(localEvent.absolutePath, "rb")

       if not file then
           logger.log("cannot open " .. localEvent.absolutePath)
           return nil
       end

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

    local filePath = fileops.removePrefix(path, 
                       event_processor.lockPrefix)
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

    local filePath = fileops.removePrefix(path,
                       event_processor.lockPrefix)
    if filePath then
        event_filter.removeIgnorePath(filePath)
    end
end

--Local function that will execute the local event.
--It makes backups(if I have time to implement) and
--returns true if the event should be sent on the network.
local function execLocalEvent(event)
    if fileops.isLockFile(event.absolutePath) then
        if event.eventType == "create" then
            --all the events on the locked file
            --will be ignored
            ignoreLockedFile(event.absolutePath)
        elseif event.eventType == "deleted" then
            --all the events on the locked file
            --will now be processed
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
        logger.log(event.absolutePath .. " open")
        conmgr.handleLocalOpenEvent(event)
        return false
    end
    
    if event.eventType == "closeWrite" then 
        logger.log(event.absolutePath .. " closeWrite event")
        local conflict = conmgr.handleLocalCloseEvent(event)
        if conflict then
            logger.log("local event conflict on file " .. event.absolutePath)
            local conflictEvents = 
                conmgr.getConflictEvents(event.absolutePath)
            _conflictFunction(event.absolutePath, conflictEvents)
            return false
        end
    end

    if event.eventType == "closeNoWrite" then
        logger.log(event.absolutePath .. " closeNoWrite event")
        local conflict = conmgr.handleLocalCloseEvent(event)
        if conflict then
            --exec events on the file
            local conflictEvents =
                conmgr.getConflictEvents(event.absolutePath)
            for _, event in pairs(conflictEvents) do
                event_processor.processNetworkEvent(event)
            end
        end
        return false
    end

    return true
end

--Local function that sends the network event using
--the broadcast mechanism
local function sendNetworkEvent(networkEvent)
   local eventJson = json.encode(networkEvent)
   logger.log("event_processor" .. eventJson)
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
       if networkEvent then
           sendNetworkEvent(networkEvent)
       end
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
    local ok = nil
    local conflict = conmgr.handleNetworkEvent(absPath)
    if conflict then
        conmgr.addConflictEvent(absPath, event)
        logger.log("conflict on file " .. absPath)
        --don't handle the event
        return
    end

    if event.eventType == "create" then
        if event.fileType == "file" then
            fileops.lockFile(absPath)
            ok = fileops.createFile(absPath)
            fileops.unlockFile(absPath)
        elseif event.fileType == "directory" then
            fileops.lockDir(absPath)
            ok = fileops.createDir(absPath)
            fileops.unlockDir(absPath)
        end
    elseif event.eventType == "deleted" then
        if event.fileType == "file" then
            fileops.lockFile(absPath)
            ok = fileops.removeFile(absPath)
            fileops.unlockFile(absPath)
        elseif event.fileType == "directory" then
            fileops.lockDir(absPath)
            ok = fileops.removeDir(absPath)
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
            ok = fileops.move(movedFromDir, absPath)
            fileops.unlockDir(movedFromDir)
            movedFromDir = ""
        end
    elseif event.eventType == "closeWrite" then
        fileops.lockFile(absPath)
        ok = fileops.writeFile(absPath, event.buffer)
        fileops.unlockFile(absPath)
    end

    if not ok then
        local errMsg = ""
        errMsg = string.format("operation %s on file %s failed",
            event.eventType, absPath)
        logger.log(errMsg)
    end
end

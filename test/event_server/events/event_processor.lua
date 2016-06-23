local json = require("json")
local fileops = require("file_operations")
local conmgr = require("events.consistency_manager")

--global table that will hold all the functions visible 
--to the application
event_processor = {}

--Module variable that holds the root directory
local rootDir = ""
--Module variable that holds a list with all the
--ignored file name prefixes. Any file or directory
--that starts with this name, will have its events
--ignored(i.e. not forwarded on the network).
local ignorePrefixes = {}

--Function that initializes the internal state of the module
function event_processor.init(argRootDir, lockPrefix)
    rootDir = argRootDir
    
    ignorePrefixes[#ignorePrefixes + 1] = "."

    fileops.init(lockPrefix)
    if lockPrefix then
        ignorePrefixes[#ignorePrefixes + 1] = lockPrefix
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

--Local function that checks if the given file must be ignored.
--If it has its name starting with one of the ignorePrefixes.
local function ignoreFile(absolutePath)
    for _, prefix in pairs(ignorePrefixes) do
        if fileops.hasPrefix(absolutePath, prefix) then
            return true
        end
    end

    return false
end

--Local function that will execute the local event.
--It makes backups(if I have time to implement) and
--returns true if the event should be sent on the network.
local function execLocalEvent(event)
    print("event_processor::execLocalEvent")
    
    if ignoreFile(event.absolutePath) then
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
            --TODO: Andrei: think of a way to solve conflicts
            print("conflict on file ", event.absolutePath)
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
function event_processor.processNetworkEvent(event)
    local absPath = fileops.absolutePath(event.relativePath, rootDir)
    if event.eventType == "create" then
        if event.fileType == "file" then
            fileops.createFile(absPath)
        elseif event.fileType == "directory" then
            fileops.createDir(absPath)
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
        elseif event.fileType == "directory" then
            fileops.move(movedFromDir, absPath)
            fileops.unlockDir(movedFromDir)
        end
    elseif event.eventType == "closeWrite" then
        local conflict = conmgr.handleNetworkCloseEvent(absPath)
        if conflict then
            --TODO: Andrei: Think of a way to solve conflicts
            print("conflict on file ", absPath)
        else
            fileops.lockFile(absPath)
            fileops.writeFile(absPath, event.buffer)
            fileops.unlockFile(absPath)
        end
    end
end

local json = require("json")
local fileops = require("file_operations")

--global table that will hold all the functions visible 
--to the application
event_processor = {}

--Module variable that holds the root directory
local rootDir = ""

--Function that initializes the internal state of the module
function event_processor.init(argRootDir)
    rootDir = argRootDir
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
    if (output.fileType == "file") and (output.eventType == "close") then
       local file = assert(io.open(localEvent.absolutePath, "rb"))
       output.buffer = file:read("*all")
       file:close()
   else
       output.buffer = ""
   end

   return output
end

--Local function that will execute the local event.
--It makes backups(if I have time to implement) and
--returns true if the event should be sent on the network.
local function execLocalEvent(event)
    --TODO: Andrei: implement
    --print("event_processor::execLocalEvent")
    local fileName = fileops.getFileName(event.absolutePath)

    --if the file is a special one(i.e. starting with ".")
    --it will not be taken into account
    if fileName and string.sub(fileName, 1, 1) == "." then
        return false
    end

    --if it is a open event, don't forward it to the other
    --users
    if event.eventType == "open" then
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
--eventType = type of event (create, deleted, open, close
--            movedTo, movedFrom)
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
--            close)
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
            fileops.removeFile(absPath)
        elseif event.fileType == "directory" then
            fileops.removeDir(absPath)
        end
    elseif event.eventType == "movedFrom" then
        if event.fileType == "file" then
            movedFromFile = absPath
        elseif event.fileType == "directory" then
            movedFromDir = absPath
        end
    elseif event.eventType == "movedTo" then
        if event.fileType == "file" then
            fileops.move(movedFromFile, absPath)
        elseif event.fileType == "directory" then
            fileops.move(movedFromDir, absPath)
        end
    elseif event.eventType == "close" then
        fileops.writeFile(absPath, event.buffer)
    end
end

local json = require("json")

--global table that will hold all the functions visible 
--to the application
event_processor = {}

--Module variable that holds the root directory
local rootDir = ""

--Function that initializes the internal state of the module
function event_processor.init(argRootDir)
    rootDir = argRootDir
end

--Local function that transforms the absolute path in a relative
--path
local function transformPath(path)
    return string.gsub(path, rootDir, "")
end

--Local function that returns the file name
--from the given path
local function getFileName(path)
    local parentDir = string.match(path, ".*/")
    if parentDir then
        local fileName = string.gsub(path, parentDir, "")
        return fileName
    else
        return nil
    end
end

--Local function that constructs a network event from the
--given local event
local function makeNetworkEvent(localEvent)
    local output = {}
    output.relativePath = transformPath(localEvent.absolutePath)
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
    local fileName = getFileName(event.absolutePath)

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
--eventType = type of event (create, modified, deleted,
--            movedTo, movedFrom)
--fileType =  type of file(file, directory)
function event_processor.processLocalEvent(event)
   local send = execLocalEvent(event)
   if send then
       local networkEvent = makeNetworkEvent(event)
       sendNetworkEvent(networkEvent)
   end
end

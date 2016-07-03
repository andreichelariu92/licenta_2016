#!/usr/bin/lua
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"
package.cpath = package.cpath .. ";/usr/local/share/lua/5.2/?.so"
local socket = require("socket")
local json = require("json")

local listOfFileEvents = {}

--Table that will be exposed to the rest of the application.
--All the interaction between the application and the passive
--front end will happen through this table
pfe_client = {}

--Function that deserializes the json file event and saves it
--in the global table
local function getFileEvent(jsonMessage)
    fileEvent = json.decode(string.sub(jsonMessage, 2, -2))
    if fileEvent then
        listOfFileEvents[#listOfFileEvents + 1] = fileEvent
    end
    return nil
end

--Function that receives a string with the json events
--separated by %% separator and uses the function
--getFileEvent to fill the global list
local function deserializeEvents(message)
    --match string between % and %
    --% has a special meaning in lua
    --so it needs to be escaped
    pattern = "(%%.-%%)"
    string.gsub(message, pattern, getFileEvent)
end

--The only public function in the module. It is a
--coroutine that waits the specified timeout and
--then returns a table containing all the file events
--received from the passive_front_end process
function pfe_client.getFileEvents(port, timeout)
    --set default parameters
    if not port then
        port = 2001
    end
    if not timeout then
        timeout = 5
    end
    --create a unix socket and set
    --it a timeout value
    local clientSocket = assert(socket.tcp())
    assert(clientSocket:connect("127.0.0.1", port))
    --logger.log("Client has connected")
    print("Client has connected\n")
    while true do
        --clear the list (remove the events
        --from the previous iteration)
        listOfFileEvents = {}
        clientSocket:settimeout(timeout, 'b')
        clientSocket:send("receiveData")
        local line, err = clientSocket:receive()
        if not err then
            deserializeEvents(line)
        else
            logger.log("no events")
        end
        coroutine.yield(listOfFileEvents)
    end
    clientSocket:close()
end

--Test function
local function consumer()
    co_eventGenerator = coroutine.create(pfe_client.getFileEvents)
    local count = 0
    while count < 10 do
        local state, fileEvents = coroutine.resume(co_eventGenerator)
        if state then
            print("count = ", count)
            for k, v in pairs(fileEvents) do
                print(v.absolutePath, v.eventType, v.fileType)
                print("\n")
            end
        end

        count  = count + 1
    end
end

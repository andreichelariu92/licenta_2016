#!/usr/bin/lua
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"
package.cpath = package.cpath .. ";/usr/local/share/lua/5.2/?.so"
socket = require("socket")
json = require("json")

listOfFileEvents = {}

function getFileEvent(jsonMessage)
    print(jsonMessage)
    fileEvent = json.decode(string.sub(jsonMessage, 2, -2))
    if fileEvent then
        listOfFileEvents[#listOfFileEvents + 1] = fileEvent
    end
    return nil
end

function deserializeEvents(message)
    --match string between % and %
    --% has a special meaning in lua
    --so it needs to be escaped
    pattern = "(%%.-%%)"
    string.gsub(message, pattern, getFileEvent)
end

function getFileEvents(port, timeout)
    --set default parameters
    if not port then
        port = 2001
    end
    if not timeout then
        timeout = 10
    end
    --create a unix socket and set
    --it a timeout value
    local clientSocket = assert(socket.tcp())
    assert(clientSocket:connect("127.0.0.1", port))
    print("Client has connected\n")
    while true do
        --clear the list (remove the events
        --from the previous iteration)
        listOfFileEvents = {}
        clientSocket:settimeout(timeout, 'b')
        clientSocket:send("receiveData")
        local line, err = clientSocket:receive()
        print("line = ", line)
        if not err then
            deserializeEvents(line)
        else
            print("error = ", err)
        end
        coroutine.yield(listOfFileEvents)
    end
    clientSocket:close()
end

function consumer()
    co_eventGenerator = coroutine.create(getFileEvents)
    local count = 0
    while count < 5 do
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

consumer()

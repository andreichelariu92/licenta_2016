#!/usr/bin/lua
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"
socket = require("socket")
json = require("json")
socket.unix = require("socket.unix")

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

function getFileEvents(path, timeout)
    --set default parameters
    if not path then
        path = "../passive_front_end/file.txt";
    end
    if not timeout then
        timeout = 10
    end
    --create a unix socket and set
    --it a timeout value
    local clientSocket = assert(socket.unix())
    assert(clientSocket:connect(path))
    print("Client has connected\n")
    while true do
        --clear the list (remove the events
        --from the previous iteration)
        listOfFileEvents = {}
        clientSocket:settimeout(timeout, 'b')
        local line, err = clientSocket:receive()
        print("line = ", line)
        if not err then
            deserializeEvents(line)
            clientSocket:send("ack")
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
    while count < 10 do
        local state, fileEvents = coroutine.resume(co_eventGenerator)
        if state then
            print("count = ", count)
            for k, v in pairs(fileEvents) do
                print(v.absolutePath, v.fileEvent, v.fileType)
                print("\n")
            end
        end

        count  = count + 1
    end
end

consumer()

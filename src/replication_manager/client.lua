#!/usr/bin/lua
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"
socket = require("socket")
json = require("json")
socket.unix = require("socket.unix")

c = assert(socket.unix())

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

assert(c:connect("./file.txt"))
line, err = c:receive()
local count = 0
while not err  and count < 3 do
    print(line)
    getFileEvent(line)
    message = string.format("count = %d\n", count)
    print(message)
    c:send(message)
    count = count + 1
    line, err = c:receive()
end
c:close()

for k, fe in pairs(listOfFileEvents) do
    print(fe.absolutePath)
    print(fe.fileType)
    print(fe.eventType)
end

#!/usr/bin/lua
--[[
--add the path of the socket library
--to the list of the lua interpreter
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"

socket = require("socket")

--set port number
port = 0
if not arg[1] then
    print("No port given as argument; 12345 is the default port\n")
    port = 12345
else
    port = arg[1]
end

--set file name
local fileName = "file1"
if not arg[2] then
    print("file1 will be sent\n")
else
    fileName = arg[2]
end

--set parity
local parity = 0
if not arg[3] then
    print("The file will be sent after the first message\n")
else
    parity = 1
end

testFile = assert(io.open("ceva_sexi", "r"))
testFileContent = testFile:read("*all")
testFile:close()

local server = assert(socket.bind("*", port))
local client = server:accept()
client:send(testFileContent)

--[[
client:settimeout(15)
local messageCount = 0
while messageCount < 2 do
    local line, err = client:receive()
    if not err then
       receivedFile = assert(io.open("Received", "w"))
       receivedFile:write(line)
       receivedFile:close()

       print("File received")
    else
        print("Error ", err)
    end
    messageCount = messageCount + 1
end
]]--

require("broadcast_mechanism")
bcast.init(12345)
local expectedClocks = 10
local start = os.clock()
while os.clock() - start < expectedClocks do
end 

bcast.addConnection("127.0.0.1", 54321, "connection2")
bcast.sendToAll("buffer", "id")

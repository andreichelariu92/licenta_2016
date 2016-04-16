#!/usr/bin/lua
package.path = package.path .. ";/usr/local/share/lua/5.2/?.lua"
socket = require("socket")
json = require("json")
socket.unix = require("socket.unix")

c = assert(socket.unix())
assert(c:connect("./file.txt"))
line, err = c:receive()
if not err then
    print(line)
    fileEvent = json.decode(line)
    print (fileEvent.path)
else
    print("err")
end
c:close()

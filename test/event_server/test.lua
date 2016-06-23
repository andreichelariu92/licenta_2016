require("clients.broadcast_mechanism_client")
local json = require("json")

function sleep(seconds)
    local start = os.clock()
    while os.clock() - start < seconds do
        --wait
    end
end

sleep(5)
bcast_client.init(54321)
bcast_client.addConnection("127.0.0.1", 12345, "connection2")

function SendEvent(localEvent)
    local jsonString = json.encode(localEvent)
    if jsonString then
        bcast_client.send({id = "id1", buffer = jsonString})
    end
end

function loadEvents(fileName)
    local file = assert(io.open(fileName, "rb"))
    local fileContent = file:read("*all")

    local func = nil
    if fileContent then
        func = load(fileContent)
    end

    return func
end

--set fileName
fileName = nil
if arg[1] then
    fileName = arg[1]
else
    print("The events will be written from input.lua")
    fileName = "input.lua"
end

--load chunk from file and execute it
func = loadEvents(fileName)
if func then
    func()
end

bcast_client.deinit()

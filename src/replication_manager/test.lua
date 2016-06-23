require("clients.broadcast_mechanism_client")
require("clients.passive_front_end_client")
require("events.event_processor")
local json = require("json")
function sleep(seconds)
    local start = os.clock()
    while os.clock() - start < seconds do
        --wait
    end
end

bcast_client.init(12345)
event_processor.init("/home/andrei/test")

sleep(5)
co_fileEvents = coroutine.create(pfe_client.getFileEvents)
local count = 0
while count < 10 do
    local state, fileEvents = coroutine.resume(co_fileEvents)
    if state then
        for k, event in pairs(fileEvents) do
            event_processor.processLocalEvent(event)
        end
    end

    local messages = bcast_client.receive(1000)
    for _, message in pairs(messages) do
        print("Network event ", message.buffer)
        local netEvent = json.decode(message.buffer)
        if netEvent then
            event_processor.processNetworkEvent(netEvent)
        end
    end
    count = count +1
end

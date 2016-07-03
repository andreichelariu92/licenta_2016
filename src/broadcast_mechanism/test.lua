require "broadcast_mechanism_client"
badParam = {}
bcast_client.init(12345)
--wait
local expectedClocks = 30
local start = os.clock()
while os.clock() - start < expectedClocks do
end
--bcast_client.addConnection("127.0.0.1", 12345, "connection1")

for messageCount = 1, 10, 1 do
    bcast_client.send({buffer = "ceva\n", id = "id"})
end
messages = bcast.deinit()
for messageIdx, message in pairs(messages) do
    print(messageIdx)
    print(message.id)
    print(message.buffer)
end

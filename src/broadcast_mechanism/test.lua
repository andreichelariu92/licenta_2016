require "broadcast_mechanism_client"
badParam = {}
bcast_client.init(54321)
--wait
local expectedClocks = 10
local start = os.clock()
while os.clock() - start < expectedClocks do
end
bcast_client.addConnection("127.0.0.1", 12345, "connection1")

messages = bcast_client.receive(5000)
for messageIdx, message in pairs(messages) do
    print(messageIdx)
    print(message.id)
    print(message.buffer)
end

messages = bcast.deinit()
for messageIdx, message in pairs(messages) do
    print(messageIdx)
    print(message.id)
    print(message.buffer)
end

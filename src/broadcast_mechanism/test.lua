require "broadcast_mechanism"
badParam = {}
bcast.init(54321)
--wait
local expectedClocks = 10
local start = os.clock()
while os.clock() - start < expectedClocks do
end
bcast.addConnection("127.0.0.1", 12345, "connection1")

messages = bcast.receiveFromAll(5000)
for messageIdx, message in pairs(messages) do
    print(messageIdx)
    print(message.id)
    print(message.buffer)
end

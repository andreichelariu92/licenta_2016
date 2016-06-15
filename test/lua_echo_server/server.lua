require("broadcast_mechanism")
bcast.init(12345)
local expectedClocks = 10
local start = os.clock()
while os.clock() - start < expectedClocks do
end 

bcast.addConnection("127.0.0.1", 54321, "connection2")
bcast.sendToAll("buffer", "id")

bcast.deinit()

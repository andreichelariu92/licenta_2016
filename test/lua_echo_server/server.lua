require("broadcast_mechanism")
bcast.init(12345)
function timeout(clocks)
    local start = os.clock()
    while os.clock() - start < clocks do
        --wait
    end
end

timeout(10)
bcast.addConnection("127.0.0.1", 54321, "connection2")
bcast.sendToAll("buffer", "id")
timeout(10)
bcast.deinit()

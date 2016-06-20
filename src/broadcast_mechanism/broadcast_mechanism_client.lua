--package.cpath = package.cpath .. ";../broadcast_mechanism/?.so"
require("broadcast_mechanism")

--Table that will be exposed to the rest of the application. All
--the interaction between the replication manager and the broadcast
--mechanism will be through the functions in this table.
bcast_client = {}

--Function that initializes the broadcast mechanism. It receives the
--port number on which future connections can be received
function bcast_client.init(port)
    bcast.init(port)
end

--Function that adds a connection to the broadcast mechanism.
--Returns false if the connection could not be established.
function bcast_client.addConnection(ip, port, connectionId)
    return bcast.addConnection(ip, port, connectionId)
end

--Function that removes a connection from the broadcast mechanism.
function bcast_client.removeConnection(connectionId)
    bcast.removeConnection(connectionId)
end

--Function that deinitializes the broadcast mechanism. Returns a
--list with all the received messages that were not yet processed
--by the client
function bcast_client.deinit()
    return bcast.deinit()
end

--Function that sends a message using the broadcast mechanism.
--It receives a table with two fields: buffer and id.
function bcast_client.send(message)
    bcast.sendToAll(message.buffer, message.id)
end

--Function that waits for the specified timeout and then reads
--all the messages from the broadcast mechanism
function bcast_client.receive(timeout)
    return bcast.receiveFromAll(timeout)
end

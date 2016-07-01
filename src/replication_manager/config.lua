--This is an example configuration file for the replication manager
--Fell free to change the parameters, but read carefully :P

--The port on which other applications will connect
bcastPort = 12345

--The absolute path to the directory that will be listened
rootDir = "/home/andrei/test"

--The timeout between polling of messages from the network
--(in milliseconds)
bcastTimeout = 2000

--The port on which the passive front end will send events
pfePort = 2001

--The IP of the passive front end
pfeIp = "127.0.0.1"

--The timeout between polling of messages from the
--passive front end(in seconds)
pfeTimeout = 2

--Function that will be used to handle the conflicts
--that happen while multiple users from the system edit
--the same file
conflictFunction = function (localPath, conflictEvents)
  local conflictPath = 
    internal.fileops.addPrefix(localPath, 
        internal.event_processor.conflictPrefix)
  internal.fileops.copyFile(localPath, conflictPath)
  
  --execute the events on the original file
  for _, event in pairs(conflictEvents) do
      print("conflict network event ",
            event.relativePath, event.eventType)
      internal.event_processor.processNetworkEvent(event)
  end
end

--List of connections with the other applications
--in the sysem
--connections = {
--  {ip = "127.0.0.1", port = 54321, connectionId = "connection1"}
--}

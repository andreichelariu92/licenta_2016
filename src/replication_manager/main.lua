logger = require("logger")
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

function loadConfigFile()
    local file = assert(io.open("config.lua"))
    local fileContent = file:read("*all")
    local configFunc = nil
    configFunc = load(fileContent)
    if configFunc then
        configFunc()
    end
end

function configureApp()
    loadConfigFile()
    --table that will hold all the configuration
    --variables
    local config = {}
    
    --configure the bcast port
    if not bcastPort then
        print("The application will listen for peers on default ",
              "port 12345")
        config.bcastPort = 12345
    else
        config.bcastPort = bcastPort
    end

    --configure root directory
    if not rootDir then
        print("The default directory will be used /home/andrei/test")
        config.rootDir = "/home/andrei/test"
    else
        config.rootDir = rootDir
    end

    --configure the bcast timeout
    if not bcastTimeout then
        print("Default broadcast timeout will be used: 2 seconds")
        config.bcastTimeout = 2000
    else
        config.bcastTimeout = bcastTimeout
    end

    --configure pfe port
    if not pfePort then
        print("The default port for listening file events ",
              " will be used 2001")
        config.pfePort = 2001
    else
        config.pfePort = pfePort
    end

    --configure pfe ip
    if not pfeIp then
        print("The ip for the passive front end is 127.0.0.1")
        config.pfeIp = "127.0.0.1"
    else
        config.pfeIp = pfeIp
    end

    --configure pfe timeout
    if not pfeTimeout then
        print("Default timeout for file events will be used: 2 sec")
        config.pfeTimeout = 2
    else
        config.pfeTimeout = 2
    end
    
    --configure conflictFunction
    if not conflictFunction then
        print("The default function for solving conflicts will",
              " be used")
        config.conflictFunction = nil
    else
        --create "internal" table, which will hold
        --all the tables used internally by the application
        internal = {}
        internal.event_processor = event_processor
        internal.fileops = require("file_operations")
        internal.json = require("json")
        internal.conmgr = require("events.consistency_manager")
        internal.event_filter = require("events.event_filter")

        config.conflictFunction = conflictFunction
    end

    --configure list of connections
    if not connections then
        print("No connections will be added")
        config.connections = {}
    else
        config.connections = connections
    end
    
    --configure the number of events
    if not nrEvents then
        print("50 events will be processed by default. -1 specifies ",
              "to process events for ever")
        config.nrEvents = 50
    else
        config.nrEvents = nrEvents
    end

    return config
end

local config = configureApp()

bcast_client.init(config.bcastPort)
event_processor.init(config.rootDir)
logger.init()

if config.conflictFunction then
    event_processor.setConflictFunction(config.conflictFunction)
end
sleep(5)
--add connections
for _, con in pairs(config.connections) do
    bcast_client.addConnection(con.ip, con.port, con.connectionId)
end

co_fileEvents = coroutine.create(pfe_client.getFileEvents)
local count = 0
while count < config.nrEvents do
    local state, fileEvents = 
        coroutine.resume(co_fileEvents, 
                         config.pfePort, 
                         config.pfeTimeout)
    if state then
        for k, event in pairs(fileEvents) do
            event_processor.processLocalEvent(event)
        end
    end

    local messages = bcast_client.receive(config.bcastTimeout)
    for _, message in pairs(messages) do
        logger.log("Network event " .. message.buffer)
        local netEvent = json.decode(message.buffer)
        if netEvent then
            event_processor.processNetworkEvent(netEvent)
        end
    end
    count = count +1
end

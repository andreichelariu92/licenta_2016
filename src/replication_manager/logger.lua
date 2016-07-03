--Table that will hold all the variables and functions
--of the module. At the end of the script, it will be
--returned to the rest of the application.
local logger = {}
logger.file = nil

function logger.init(logFile)
    if not logFile then
        logFile = "logfile.lua"
    end
    
    logger.file = assert(io.open(logFile, "a+"))
end

local function getInfo(level)
    local info = debug.getinfo(level, "nl")
    if info then
        return info.name, info.currentline
    end
end

function logger.log(message)
    if not message then
        --do nothing
        return
    end
    
    local name, line = getInfo(3)
    if not name then
        name = "global scope"
    end

    local logMessage = 
        string.format("%s : %d  %s\n", name, line, message)
    logger.file:write(logMessage)
    logger.file:flush()
end

local function testFunc()
    logger.log("Mesaj de test")
end

return logger


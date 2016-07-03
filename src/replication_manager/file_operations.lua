--The table that will contain all the public functions
--that can be used by the rest of the application
local file_operations = {}

--Module variable that will hold the prefix added to
--the file or to the directory that will be locked.
--See lock files on web for more information.
local lockPrefix = ""

--Function that initialized the variables needed by
--this module
function file_operations.init(argLockPrefix)
    lockPrefix = argLockPrefix
end

function file_operations.createFile(path)
    local command = "touch " .. path
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.createDir(path)
    local command = "mkdir " .. path
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.removeFile(path)
    local command = "rm -f " .. path
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.removeDir(path)
    local command = "rm -Rf " .. path
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.copyFile(source, destination)
    local command = "cp " .. source .. " " .. destination
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.move(source, destination)
    if source == "" or destination == "" then
        --do nothing
        return
    end

    local command = "mv " .. source .. " " .. destination
    logger.log(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.writeFile(path, buffer)
   local file = io.open(path, "wb")
   if not file then
       return false
   end

   output = nil
   if not file:write(buffer) then
       output = false
   else
       output = true
   end

   file:close()
   return output
end

--Function that transforms the absolute path in a relative path
function file_operations.relativePath(absPath, rootDir)
    return string.gsub(absPath, rootDir, "")
end

--Function that returns the file name
--from the given path and the parent dir
function file_operations.getFileName(path)
    local parentDir = string.match(path, ".*/")
    if parentDir then
        local fileName = string.gsub(path, parentDir, "")
        return fileName, parentDir
    else
        return nil
    end
end

--Function that returns true if the file or directory
--at the given path has its name starting with the
--specified prefix.
function file_operations.hasPrefix(path, prefix)
    local fileName = file_operations.getFileName(path)
    if fileName then
        --true specifies that prefix is not regex
        local startIdx = string.find(fileName, prefix, 1, true)
        if startIdx == 1 then
            return true
        else
            return false
        end
    else
        return false
    end
end

--Function that forms the absolutePath from the relative one and
--the rootDir
function file_operations.absolutePath(relPath, rootDir)
    return (rootDir .. relPath)
end

--Function that adds the spefied prefix to the path and then
--returns the new path
function file_operations.addPrefix(path, prefix)
    local fileName, parentDir = file_operations.getFileName(path)
    if fileName and parentDir then
        fileName = prefix .. fileName
        return (parentDir .. fileName)
    else
        return nil
    end
end

--Function that removes the specified prefix from the given
--path. If the file at the specified path does not have the
--prefix, the function returns nil.
function file_operations.removePrefix(path, prefix)
    if not file_operations.hasPrefix(path, prefix) then
        return nil
    end
    
    local fileName, parentDir = file_operations.getFileName(path)
    if fileName and parentDir then
        --replace the first occurance of prefix with
        --the empty string
        fileName = string.gsub(fileName, prefix, "", 1)
        return (parentDir .. fileName)
    else
        return nil
    end
end

--Local function that creates the lock path for a file or
--directory based on the current path.
--For example: /home/andrei/file -> /home/andrei/~file
local function getLockPath(path)
    return file_operations.addPrefix(path, lockPrefix)
end

--Function that creates a lock file for the file at the given
--path.
function file_operations.lockFile(path)
    local lockPath = getLockPath(path)
    if lockPath then
        return file_operations.createFile(lockPath)
    else
        return false
    end
end

--Function that removes a lock file for the file at the given
--path.
function file_operations.unlockFile(path)
    local lockPath = getLockPath(path)
    if lockPath then
        return file_operations.removeFile(lockPath)
    else
        return false
    end
end

--Funnction that creates a lock for the directory at
--the given path.
function file_operations.lockDir(path)
    local lockPath = getLockPath(path)
    if lockPath then
        return file_operations.createFile(lockPath)
    else
        return false
    end
end

--Function that removes a lock for the directory at
--the given path.
function file_operations.unlockDir(path)
    local lockPath = getLockPath(path)
    if lockPath then
        return file_operations.removeFile(lockPath)
    else
        return false
    end
end

--Function that returns true if the specified path is
--for a lock file
function file_operations.isLockFile(path)
    return file_operations.hasPrefix(path, lockPrefix)
end
return file_operations

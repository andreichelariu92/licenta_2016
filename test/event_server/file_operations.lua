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
    if argLockPrefix then
        lockPrefix = argLockPrefix
    else
        lockPrefix = "~"
    end
end

function file_operations.createFile(path)
    local command = "touch " .. path
    print(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.createDir(path)
    local command = "mkdir " .. path
    print(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.removeFile(path)
    local command = "rm -f " .. path
    print(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.removeDir(path)
    local command = "rm -Rf " .. path
    print(command)
    if os.execute(command) then
        return true
    else
        return false
    end
end

function file_operations.move(source, destination)
    local command = "mv " .. source .. " " .. destination
    print(command)
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
--from the given path
function file_operations.getFileName(path)
    local parentDir = string.match(path, ".*/")
    if parentDir then
        local fileName = string.gsub(path, parentDir, "")
        return fileName
    else
        return nil
    end
end

--Function that forms the absolutePath from the relative one and
--the rootDir
function file_operations.absolutePath(relPath, rootDir)
    return (rootDir .. relPath)
end

--Local function that creates the lock path for a file or
--directory based on the current path.
--For example: /home/andrei/file -> /home/andrei/~file
local function getLockPath(path)
    local parentDir = string.match(path, ".*/")
    if parentDir then
        local fileName = string.gsub(path, parentDir, "")
        fileName = lockPrefix .. fileName
        print("getLockPath path = ", path, " lockFile = ", fileName)
        return (parentDir .. fileName)
    else
        return nil
    end
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
        return file_operations.createDir(lockPath)
    else
        return false
    end
end

--Function that removes a lock for the directory at
--the given path.
function file_operations.unlockDir(path)
    local lockPath = getLockPath(path)
    if lockPath then
        return file_operations.removeDir(lockPath)
    else
        return false
    end
end

--Function that returns true if the file or directory
--at the given path has its name starting with the
--specified prefix.
function file_operations.hasPrefix(path, prefix)
    local fileName = file_operations.getFileName(path)
    if fileName then
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

return file_operations

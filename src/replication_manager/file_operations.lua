--The table that will contain all the public functions
--that can be used by the rest of the application
local file_operations = {}

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
return file_operations

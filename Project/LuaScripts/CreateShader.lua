-- create_shader.lua
function CreateShader(sourceFileName, destinationFolder, destinationFileName)
    local sourceFile = io.open(sourceFileName, "rb")

    if not sourceFile then
        print("Failed to open:" .. sourceFileName .. " the source file for reading.")
        return
    end

    local content = sourceFile:read("*all")
    sourceFile:close()

    local destinationPath = destinationFolder .. "/" .. destinationFileName
    local destinationFile = io.open(destinationPath, "wb")

    if not destinationFile then
        print("Failed to open the destination file for writing: " .. destinationPath)
        return
    end

    destinationFile:write(content)
    destinationFile:close()

    print("Shader file copied successfully to: " .. destinationPath)
end
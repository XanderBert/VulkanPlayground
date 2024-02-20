-- create_shader.lua
function CreateShader(sourceFileName, destinationFolder, destinationFileName)
    

    local sourceFile = io.open(sourceFileName, "rb")


    if not sourceFile then
        print("Failed to open the source file for reading.")
        return
    end

    local content = sourceFile:read("*all")
    sourceFile:close()

    local destinationPath = destinationFolder .. "/" .. destinationFileName .. ".frag"
    local destinationFile = io.open(destinationPath, "wb")

    if not destinationFile then
        print("Failed to open the destination file for writing.")
        return
    end

    destinationFile:write(content)
    destinationFile:close()

    print("Shader file copied successfully to: " .. destinationPath)
end
#pragma once 
#include <string>
#include <vector>

inline FilesystemModule& FS = FilesystemModule::getInstance();

enum class FResult : uint8_t
{
    SUCCESS,
    ALREADY_EXISTS,
    INVALID_PATH,
    UNDEFIND
};

class FilesystemModule
{
    FilesystemModule() = default;
   
public:
    ~FilesystemModule() = default;
    using constr = const std::string&;
    static FilesystemModule& getInstance()
    {
        static FilesystemModule FilesystemModule;
        return FilesystemModule;
    }

    void startup(){}
    void shutDown(){}
    
    FResult createFile(constr dirPath, constr fileName);
    FResult createDirectory(constr dirPath, constr dirName);

    FResult deleteFile(constr filePath);
    FResult duplicateFileInDirectory(constr filePath);
    FResult moveFileToDirectory(constr filePath, constr destPath);
    FResult copyAbsolutePath(constr filePath);
    FResult copyRelativePath(constr filePath);

    FResult writeTextFile(constr filePath, constr content);
    FResult appendToTextFile(constr filePath, constr content);
    FResult writeBinaryFile(constr filePath, const std::vector<uint8_t>& data);
    FResult appendToBinaryFile(constr filePath, const std::vector<uint8_t>& data);

    std::vector<uint8_t> readBinaryFile(constr filePath);
    std::string readTextFile(constr filePath);

    std::string getFileName(constr filePath);
    std::string getFileExtensions(constr filePath);
    size_t getFileSize(constr filePath);

    bool isPathExists(constr path);
};

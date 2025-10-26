#pragma once 
#include <EngineMinimal.h>

namespace ProjectModule
{
	class ProjectModule;
}

namespace Logger
{
	class Logger;
}

namespace FilesystemModule
{
	class DirectoryIterator;

	enum class FResult : uint8_t
	{
		SUCCESS,
		ALREADY_EXISTS,
		INVALID_PATH,
		UNDEFIND
	};

	enum class DirContentType : uint8_t
	{
		ALL,
		FILES,
		FOLDERS,
	};

	struct ContentSearchFilter
	{
		DirContentType contentType = DirContentType::ALL;
		std::optional<std::vector<std::string>> fileExtensions;
		std::optional<std::string> filter;
	};

	class FilesystemModule : public IModule
	{
		ProjectModule::ProjectModule* m_projectModule;
	public:
		using constr = const std::string&;

		void startup() override;
		void shutdown() override;

		FResult createFile(constr dirPath, constr fileName);
		FResult createDirectory(constr dirPath, constr dirName);
		DirectoryIterator createDirectoryIterator();

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

		std::string getEngineAbsolutePath(constr relativePath);
		std::string getCurrentPath();
		std::string getFileName(constr filePath);
		std::string getFileExtensions(constr filePath);
		std::string getRelativeToTheResources(constr filePath);
		size_t getFileSize(constr filePath);
		std::vector <std::string> getDirectoryContents(constr dirPath, ContentSearchFilter filter);
		uint64_t getFolderModificationTime(constr folderPath);

		bool isPathExists(constr path);
		bool isFile(constr path);
		bool isDirectory(constr path);
	};
}

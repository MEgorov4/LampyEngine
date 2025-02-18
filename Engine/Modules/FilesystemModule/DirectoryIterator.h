#pragma once 

#include <string>
#include <optional>
#include <vector>
#include <filesystem>

#include "FilesystemModule.h"

namespace fs = std::filesystem;

class DirectoryIterator
{
	std::optional<uint64_t> m_dirLastEditTime;
	fs::path m_rootPath;
	fs::path m_currentPath;
public:
	using constr = const std::string&;

	/// <summary>
	/// Default constructor. Iterator begins from default resources path.
	/// </summary>
	DirectoryIterator();
	
	/// <summary>
	/// Constructor with initialisation from startup path.
	/// </summary>
	/// <param name="startupPath">Startup path</param>
	DirectoryIterator(constr startupPath);

	FResult stepIntoRoot();
	FResult stepIntoParent();
	FResult stepIntoFolder(constr folderName);

	std::string getCurrentDirName();
	std::string getCurrentDir();
	std::string getCurrentDirWithAppend(constr appendName);
	std::vector<std::string> getCurrentDirContents(const ContentSearchFilter& filter);

	bool isRootPath();
	bool isCurrentDirChanged();
};
#include "DirectoryIterator.h"
#include <cassert>

#include "../ProjectModule/ProjectModule.h"
#include "../LoggerModule/Logger.h"
#include "../../EngineContext/CoreGlobal.h"

namespace FilesystemModule
{
	DirectoryIterator::DirectoryIterator(FilesystemModule* filesystemModule
		, const std::string& rootPath
		, const std::string& currentPath)
		: m_filesystemModule(filesystemModule)
		, m_logger(GCM(Logger::Logger))
		, m_projectModule(GCM(ProjectModule::ProjectModule))
	{
		assert(m_filesystemModule);
		m_rootPath = !filesystemModule->isPathExists(rootPath) ? m_projectModule->getProjectConfig().getResourcesPath() : rootPath;
		m_currentPath = !filesystemModule->isPathExists(rootPath) ? m_rootPath : currentPath;
	}


	FResult DirectoryIterator::stepIntoRoot()
	{
		m_currentPath = m_rootPath;
		m_dirLastEditTime = std::nullopt;
		return FResult::SUCCESS;
	}

	FResult DirectoryIterator::stepIntoParent()
	{
		if (m_currentPath != m_rootPath)
		{
			m_currentPath = m_currentPath.parent_path();
			m_dirLastEditTime = std::nullopt;
			return FResult::SUCCESS;
		}
		m_logger->log(Logger::LogVerbosity::Error, "Invalid path to step", "FilesystemModule_DirectoryIterator");
		return FResult::INVALID_PATH;
	}


	FResult DirectoryIterator::stepIntoFolder(constr folderName)
	{
		fs::path resultPath = m_currentPath / fs::path(folderName);

		if (!m_filesystemModule->isPathExists(resultPath.string()))
		{
			m_logger->log(Logger::LogVerbosity::Error, "Invalid path to step: " + resultPath.string(), "FilesystemModule_DirectoryIterator");
			return FResult::INVALID_PATH;
		}

		m_currentPath = resultPath;
		m_dirLastEditTime = std::nullopt;
		return FResult::SUCCESS;
	}

	std::string DirectoryIterator::getCurrentDirName()
	{
		return m_currentPath.filename().string();
	}

	std::string DirectoryIterator::getCurrentDir()
	{
		return m_currentPath.string();
	}

	std::string DirectoryIterator::getCurrentDirWithAppend(constr appendName)
	{
		return (m_currentPath / fs::path(appendName)).string();
	}

	std::vector<std::string> DirectoryIterator::getCurrentDirContents(const ContentSearchFilter& filter)
	{
		return m_filesystemModule->getDirectoryContents(m_currentPath.string(), filter);
	}

	bool DirectoryIterator::isRootPath()
	{
		return m_currentPath == m_rootPath;
	}

	bool DirectoryIterator::isCurrentDirChanged()
	{
		uint64_t lastTime = m_filesystemModule->getFolderModificationTime(m_currentPath.string());

		if (!m_dirLastEditTime)
		{
			m_dirLastEditTime = lastTime;
			return true;
		}

		if (m_dirLastEditTime != lastTime || m_dirLastEditTime == 0)
		{
			m_dirLastEditTime = lastTime;
			return true;
		}
		return false;
	}
}

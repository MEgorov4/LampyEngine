#include "DirectoryIterator.h"

#include "../ProjectModule/ProjectModule.h"

DirectoryIterator::DirectoryIterator() : m_rootPath(PM.getProjectConfig().getResourcesPath()), m_currentPath(m_rootPath)
{

}

DirectoryIterator::DirectoryIterator(constr startupPath) : m_rootPath(PM.getProjectConfig().getResourcesPath()), m_currentPath(startupPath)
{

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
	
	LOG_ERROR("DirrectoryIterator:stepIntoParent: invalid path to step: " + m_currentPath.parent_path().string());
	return FResult::INVALID_PATH;
}


FResult DirectoryIterator::stepIntoFolder(constr folderName)
{
	fs::path resultPath = m_currentPath / fs::path(folderName);
	
	if (!FS.isPathExists(resultPath.string()))
	{
		LOG_ERROR("DirrectoryIterator:stepIntoFolder: invalid path to step: " + resultPath.string());
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
	return FilesystemModule::getInstance().getDirectoryContents(m_currentPath.string(), filter);
}

bool DirectoryIterator::isRootPath()
{
	return m_currentPath == m_rootPath;
}

bool DirectoryIterator::isCurrentDirChanged()
{
	uint64_t lastTime = FS.getFolderModificationTime(m_currentPath.string());
	
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



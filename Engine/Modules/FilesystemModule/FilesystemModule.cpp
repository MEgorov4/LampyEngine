#include "FilesystemModule.h"
#include <filesystem>
#include <fstream>
#include <clip.h>

#include "../LoggerModule/Logger.h"
#include "../ProjectModule/ProjectModule.h"

namespace fs = std::filesystem;

FResult FilesystemModule::deleteFile(constr path)
{
	if (!isPathExists(path))
	{
		LOG_ERROR(std::format("FilesystemModule:deleteFile: invalid path file: {}", path));
		return FResult::INVALID_PATH;
	}

	if (!fs::remove(path))
	{
		LOG_ERROR(std::format("FilesystemModule:deleteFile: failed to remove file: {}", path));
		return FResult::UNDEFIND;
	}

	return FResult::SUCCESS;
}

FResult FilesystemModule::duplicateFileInDirectory(constr path)
{
	if (!isPathExists(path))
	{
		LOG_ERROR(std::format("FilesystemModule:deleteFile: invalid path file: {}", path));
		return FResult::INVALID_PATH;
	}

	fs::path originalPath(path);
	fs::path directory = originalPath.parent_path();
	std::string baseName = originalPath.stem().string();
	std::string extension = originalPath.extension().string();

	fs::path newFilePath = directory / (baseName + extension);

	int index = 1;

	while (isPathExists(newFilePath.string()))
	{
		newFilePath = directory / (baseName + "_" + std::to_string(index) + extension);
		index++;
	}
	
	if (!fs::copy_file(path, newFilePath, fs::copy_options::overwrite_existing))
	{
		LOG_ERROR(std::format("FilesystemModule:duplicateFileInDirectory: failed to duplicate file in directory {}", path));
		return FResult::UNDEFIND;
	}
	return FResult::SUCCESS;
}

FResult FilesystemModule::moveFileToDirectory(constr filePath, constr destinationPath)
{
	if (!isPathExists(filePath) || !isPathExists(destinationPath))
	{
		LOG_ERROR("FilesystemModule:moveFileToDirectory: invalid file path or destination path");
		return FResult::INVALID_PATH;
	}

	fs::path source(filePath);
	fs::path destination = fs::path(destinationPath) / source.filename();

	if (!fs::copy_file(source, destination, fs::copy_options::overwrite_existing))
	{
		LOG_ERROR("FilesystemModule:moveFileToDirectory: failed to copy file: " + filePath);
		return FResult::UNDEFIND;
	}

	if (!fs::remove(source))
	{
		LOG_ERROR("FilesystemModule:moveFileToDirectory: failed to remove: " + filePath);
		return FResult::UNDEFIND;
	}

	return FResult::SUCCESS;
}

FResult FilesystemModule::copyAbsolutePath(constr filePath)
{
	if (!clip::set_text(filePath))
	{
		LOG_ERROR(std::format("FilesystemModule:CopyAbsolutePath: failed to copy path: {}", filePath));

		return FResult::UNDEFIND;
	}
	return FResult::SUCCESS;
}

FResult FilesystemModule::copyRelativePath(constr filePath)
{
	std::string relativeFilePath = fs::relative(filePath, ProjectModule::getInstance().getProjectConfig().getResourcesPath()).string();

	if (!clip::set_text(relativeFilePath))
	{
		LOG_ERROR(std::format("FilesystemModule:CopyRelativePath: failed to copy path: {}", relativeFilePath));
		return FResult::UNDEFIND;
	}
	return FResult::SUCCESS;
}

std::vector<uint8_t> FilesystemModule::readBinaryFile(constr filePath)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file) {
		LOG_ERROR(std::format("FilesystemModule:readBinaryFile: failed to read binary file: {}", filePath));
		return std::vector<uint8_t>();
	}

	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(fileSize);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	return buffer;
}

std::string FilesystemModule::readTextFile(constr filePath)
{
	std::ifstream file(filePath, std::ios::in);
	if (!file){
		LOG_ERROR(std::format("FilesystemModule:readTextFile: failed to read text file: {}", filePath));
		return "";
	}

	std::ostringstream buffer;

	buffer << file.rdbuf();
	return buffer.str();
}

std::string FilesystemModule::getFileName(constr filePath)
{
	return fs::path(filePath).filename().string();
}

std::string FilesystemModule::getFileExtensions(constr filePath)
{
	if (!isPathExists(filePath))
	{
		LOG_ERROR(std::format("FilesystemModule:getFileExtensions: file does not exist: {}", filePath));
		return "";
	}
	return fs::path(filePath).extension().string();
}

size_t FilesystemModule::getFileSize(constr filePath)
{
	if (!isPathExists(filePath))
	{
		LOG_ERROR(std::format("FilesystemModule:getFileSize: file does not exist: {}", filePath));
		return 0;
	}
	return fs::file_size(filePath);
}

bool FilesystemModule::isPathExists(constr path)
{
	return fs::exists(path);
}

FResult FilesystemModule::createFile(constr dirPath, constr fileName)
{
	if (!isPathExists(dirPath))
	{
		return FResult::INVALID_PATH;
	}

	fs::path dir(dirPath);
	fs::path fName(fileName);
	fs::path outFilePath = dirPath / fName;
	std::ofstream file(outFilePath);
	
	if (!file.is_open())
	{
		return FResult::UNDEFIND;
	}
	file.close();

	return FResult::SUCCESS;
}

FResult FilesystemModule::createDirectory(constr dirPath, constr dirName)
{
	std::error_code ec;

	if (isPathExists(dirPath))
	{
		return FResult::ALREADY_EXISTS;
	}

	if (fs::create_directories(dirPath, ec))
	{
		return FResult::SUCCESS;
	}

	LOG_ERROR(std::format("FilesystemModule:createDirectory: failed to create directory: {}", dirPath));
	return FResult::UNDEFIND;
}

FResult FilesystemModule::writeTextFile(constr filePath, constr content)
{
	std::ofstream file(filePath, std::ios::out);

	if (!file)
	{
		LOG_ERROR(std::format("FilesystemModule:writeTextFile: failed to open file: {}", filePath));
		return FResult::UNDEFIND;
	}

	file << content;
	file.close();

	return FResult::SUCCESS;
}

FResult FilesystemModule::appendToTextFile(constr filePath, constr content)
{
	std::ofstream file(filePath, std::ios::app);
	if (!file)
	{
		LOG_ERROR(std::format("FilesystemModule:appendToTextFile: failed to open file: {}", filePath));
		return FResult::UNDEFIND;
	}

	file << content << std::endl;
	file.close();

	return FResult::SUCCESS;
}

FResult FilesystemModule::writeBinaryFile(constr filePath, const std::vector<uint8_t>& data)
{
	std::ofstream file(filePath, std::ios::binary | std::ios::out);
	if (!file) {
		LOG_ERROR(std::format("FilesystemModule:writeBinaryFile: failed to open file: {}", filePath));
		return FResult::UNDEFIND;
	}

	file.write(reinterpret_cast<const char*>(data.data()), data.size());
	file.close();
	return FResult::SUCCESS;
}

FResult FilesystemModule::appendToBinaryFile(constr filePath, const std::vector<uint8_t>& data)
{
	std::ofstream file(filePath, std::ios::binary | std::ios::app);
	if (!file)
	{
		LOG_ERROR(std::format("FilesystemModule:createFileWithSize: failed to create file: {}", filePath));
		return FResult::UNDEFIND;
	}
	
	file.write(reinterpret_cast<const char*>(data.data()), data.size());
	file.close();

	return FResult::SUCCESS;
}

#pragma once

#include <string>

class BaseResource
{
public:
	BaseResource(const std::string& path) { GUID = path; };

	std::string getGUID() const;
	void setGUID(std::string& newGUID);
protected:
	std::string GUID;
};
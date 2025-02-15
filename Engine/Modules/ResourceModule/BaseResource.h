#pragma once

#include <string>

class BaseResource
{
public:
	BaseResource() {};

	std::string getGUID() const;
	void setGUID(std::string& newGUID);
protected:
	std::string GUID;
};
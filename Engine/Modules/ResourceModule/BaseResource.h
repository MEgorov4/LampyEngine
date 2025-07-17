#pragma once

#include <string>
namespace ResourceModule
{
	class BaseResource
	{
	public:
		BaseResource(const std::string& path) { GUID = path; }

		std::string getGUID() const;
		void setGUID(const std::string& newGUID);
	protected:
		std::string GUID;
	};
}
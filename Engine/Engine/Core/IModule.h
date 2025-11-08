#pragma once 

namespace EngineCore::Base
{
	class IModule
	{
	public:
		virtual void startup() = 0;
		virtual void shutdown() = 0;
		virtual ~IModule() = default;
	};
}
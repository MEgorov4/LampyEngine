#pragma once 
#include <cstdint>

namespace EngineCore::Foundation
{
	enum class LogVerbosity : uint8_t
	{
		Verbose,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};
}
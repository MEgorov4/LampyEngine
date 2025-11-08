#pragma once
#include "ContextLocator.h"


namespace EngineCore::Base
{
	class Context final
	{
	public:
		Context() = delete;

		static void SetLocator(ContextLocator* locator)
		{
			s_activeLocator = locator;
		}

		static ContextLocator& Locator()
		{
			if (!s_activeLocator)
				throw std::runtime_error("Context::Locator(): no active context");
			return *s_activeLocator;
		}

		template<typename T>
		static T& Get()
		{
			return Locator().get<T>();
		}

	private:
		inline static ContextLocator* s_activeLocator = nullptr;
	};
}

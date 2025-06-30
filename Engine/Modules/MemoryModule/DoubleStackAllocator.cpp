/*
#include "DoubleStackAllocator.h"

#include <memory>
#include <string>
#include "../../Modules/LoggerModule/Logger.h"

DoubleStackAllocator::DoubleStackAllocator(size_t size) : m_totalSize(size)
{
	m_start = malloc(size);
	m_end = static_cast<char*>(m_start) + m_totalSize;

	reset();
}

DoubleStackAllocator::~DoubleStackAllocator()
{
	uintptr_t start = reinterpret_cast<uintptr_t>(m_start);
	uintptr_t current = reinterpret_cast<uintptr_t>(m_current);

	uintptr_t end = reinterpret_cast<uintptr_t>(m_end);
	uintptr_t reverse = reinterpret_cast<uintptr_t>(m_reverse);

	LOG_INFO("MemoryModule: FREE START STACK " + std::to_string(current - start) + "-----------------");
	LOG_INFO("MemoryModule: FREE END STACK " + std::to_string(end - reverse) + "-----------------");
	free(m_start);
}

void* DoubleStackAllocator::allocateStart(size_t size)
{
	if (static_cast<char*>(m_current) + size <= static_cast<char*>(m_reverse))
	{
		void* result = m_current;
		m_current = static_cast<char*>(m_current) + size;

		return result;
	}

	return nullptr;
}

void* DoubleStackAllocator::allocateEnd(size_t size)
{
	if (static_cast<char*>(m_reverse) - size >= static_cast<char*>(m_current))
	{
		m_reverse = static_cast<char*>(m_reverse) - size;

		return m_reverse;
	}

	return nullptr;
}

void DoubleStackAllocator::reset()
{
	m_current = m_start;
	m_reverse = m_end;
}
*/

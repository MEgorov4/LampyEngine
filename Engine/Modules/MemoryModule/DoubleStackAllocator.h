#pragma once

class DoubleStackAllocator
{
public:
	DoubleStackAllocator() {};
	DoubleStackAllocator(size_t size);
	~DoubleStackAllocator();

	void* allocateStart(size_t size);
	void* allocateEnd(size_t size);

	void reset();
private:
	size_t m_totalSize;
	void* m_start = nullptr;
	void* m_end = nullptr;
	void* m_current = nullptr;
	void* m_reverse = nullptr;
};
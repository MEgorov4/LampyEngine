/*#pragma once

class PoolAllocator
{
private:
	struct Chunk
	{
		Chunk* next;
	};

public:
	PoolAllocator() = default;
	PoolAllocator(size_t objectSize, size_t chunksPerBlock, void* placement = nullptr);
	~PoolAllocator();

	void* allocate();
	void deallocate(void* ptr);
private:
	size_t chunkSize;
	size_t m_chunksPerBlock;
	Chunk* m_alloc = nullptr;
	void* memoryPool;

	Chunk* allocateBlock(size_t size);
	void placementBlock(size_t size);

	bool m_ownsMemory = false;
};*/
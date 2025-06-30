/*
#include "PoolAllocator.h"

#include <string>
#include "../LoggerModule/Logger.h"

PoolAllocator::PoolAllocator(size_t objectSize, size_t chunksPerBlock, void* placement) : m_chunksPerBlock(chunksPerBlock)
{
	chunkSize = (objectSize < sizeof(Chunk)) ? sizeof(Chunk) : objectSize;

	if (placement)
	{
		memoryPool = placement;
		placementBlock(chunkSize);
		m_ownsMemory = false;
	}
	else
	{
		m_alloc = allocateBlock(chunkSize);
		m_ownsMemory = true;
	}
}

PoolAllocator::~PoolAllocator()
{
	if (m_ownsMemory)
	{
		free(memoryPool);
	}
}

void* PoolAllocator::allocate()
{
	if (m_alloc == nullptr)
	{
		return nullptr;
	}
	
	Chunk* freeChunck = m_alloc;
	m_alloc = m_alloc->next;

	uintptr_t address = reinterpret_cast<uintptr_t>(freeChunck);
	LOG_INFO("MemoryModule: Allocated " + std::to_string(address) + " chunks):");
	return freeChunck;
}

void PoolAllocator::deallocate(void* ptr)
{
	if (!ptr) return;

	Chunk* allocatedChunk = static_cast<Chunk*>(ptr);
	allocatedChunk->next = m_alloc;
	m_alloc = allocatedChunk;

	uintptr_t address = reinterpret_cast<uintptr_t>(allocatedChunk);
	LOG_INFO("MemoryModule: Deallocated " + std::to_string(address) + " chunks):");
}

PoolAllocator::Chunk* PoolAllocator::allocateBlock(size_t size)
{
	LOG_INFO("MemoryModule: Allocating block (" + std::to_string(m_chunksPerBlock) + " chunks):");

	size_t blockSize = m_chunksPerBlock * size;

	memoryPool = malloc(blockSize);
	if (Chunk* blockBegin = static_cast<Chunk*>(memoryPool))
	{
		Chunk* chunk = blockBegin;
		for (int i = 0; i < m_chunksPerBlock - 1; ++i)
		{
			chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(chunk) + size);
			chunk = chunk->next;
		}
		chunk->next = nullptr;

		return blockBegin;
	}

	return nullptr;
}

void PoolAllocator::placementBlock(size_t size)
{
	if (Chunk* blockBegin = static_cast<Chunk*>(memoryPool))
	{
		Chunk* chunk = blockBegin;
		for (int i = 0; i < m_chunksPerBlock - 1; ++i)
		{
			chunk->next = reinterpret_cast<Chunk*>(reinterpret_cast<char*>(chunk) + size);
			chunk = chunk->next;
		}
		chunk->next = nullptr;

		m_alloc = blockBegin;
	}
}
*/

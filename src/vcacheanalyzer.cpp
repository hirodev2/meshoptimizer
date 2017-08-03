// This file is part of meshoptimizer library; see meshoptimizer.hpp for version/license details
#include "meshoptimizer.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

namespace meshopt
{

static VertexCacheStatistics analyzeVertexCacheFIFO(const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	assert(index_count % 3 == 0);
	assert(cache_size >= 3);

	VertexCacheStatistics result = {};

	std::vector<unsigned int> cache_timestamps(vertex_count, 0);
	unsigned int timestamp = cache_size + 1;

	for (size_t i = 0; i < index_count; ++i)
	{
		unsigned int index = indices[i];
		assert(index < vertex_count);

		if (timestamp - cache_timestamps[index] > cache_size)
		{
			// cache miss
			cache_timestamps[index] = timestamp++;
			result.vertices_transformed++;
		}
	}

	result.acmr = index_count == 0 ? 0 : float(result.vertices_transformed) / float(index_count / 3);
	result.atvr = vertex_count == 0 ? 0 : float(result.vertices_transformed) / float(vertex_count);

	return result;
}

static VertexCacheStatistics analyzeVertexCacheLRU(const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	assert(index_count % 3 == 0);
	assert(cache_size >= 3);

	VertexCacheStatistics result = {};

	std::vector<unsigned int> cache;
	cache.reserve(cache_size + 1);

	for (size_t i = 0; i < index_count; ++i)
	{
		unsigned int index = indices[i];
		assert(index < vertex_count);

		std::vector<unsigned int>::iterator it = std::find(cache.begin(), cache.end(), index);

		if (it == cache.end())
			result.vertices_transformed++;
		else
			cache.erase(it);

		cache.push_back(index);

		if (cache.size() > cache_size)
			cache.erase(cache.begin(), cache.end() - cache_size);
	}

	result.acmr = index_count == 0 ? 0 : float(result.vertices_transformed) / float(index_count / 3);
	result.atvr = vertex_count == 0 ? 0 : float(result.vertices_transformed) / float(vertex_count);

	return result;
}

VertexCacheStatistics analyzeVertexCache(const unsigned int* indices, size_t index_count, size_t vertex_count, unsigned int cache_size)
{
	if (cache_size == 0)
		return analyzeVertexCacheLRU(indices, index_count, vertex_count, 16);
	else
		return analyzeVertexCacheFIFO(indices, index_count, vertex_count, cache_size);
}

} // namespace meshopt

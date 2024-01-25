#include "SizedQueue.h"

#include <algorithm>


SizedQueue::SizedQueue(const size_t size)
{
	max_size = size;
}


SizedQueue::~SizedQueue()
{
}


size_t SizedQueue::putItems(const size_t num, const int16_t* buffer)
{
	data.insert(data.end(), buffer, buffer + num);
	if (max_size < data.size())
		data.erase(data.begin(), data.begin() + data.size() - max_size);

	return std::min(num, max_size);
}


size_t SizedQueue::getItems(const size_t num, int16_t* array)
{
	size_t num_copy = std::min(num, data.size());
	std::copy_n(data.begin(), num_copy, array);
	data.erase(data.begin(), data.begin() + num_copy);

	return num_copy;
}


size_t SizedQueue::size() { return data.size(); }
void SizedQueue::clear() { data.clear(); }

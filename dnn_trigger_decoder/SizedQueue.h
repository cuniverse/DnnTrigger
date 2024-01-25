#ifndef __LVCSR_CLIENT_SIZED_QUEUE_H__
#define __LVCSR_CLIENT_SIZED_QUEUE_H__

#include <stdint.h>
#include <stdlib.h>

#include <deque>


class SizedQueue
{
public:
	SizedQueue(const size_t size);
	~SizedQueue();

	size_t putItems(const size_t num, const int16_t* buffer);
	size_t getItems(const size_t num, int16_t* buffer);
	size_t size();
	void clear();

private:
	std::deque<int16_t> data;
	size_t max_size;
};

#endif	// __LVCSR_CLIENT_SIZED_QUEUE_H__

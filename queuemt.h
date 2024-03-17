#ifndef QUEUEMT_H
#define QUEUEMT_H

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
template <typename T>
class queuemt
{
private:
	std::queue<T> data_queue;
	mutable std::mutex queue_mutex;
	std::condition_variable data_cond;
	size_t maxsize;
public:
	queuemt();
	queuemt(size_t set_maxsize);
	bool try_push(const T &new_value);
	bool try_push(T &&new_value);
	// bool wait_and_push(const T& new_value);
	// bool wait_and_push(T&& new_value);
	bool try_pop(T &value);
};

template <typename T>
queuemt<T>::queuemt()
	: maxsize(0)
{
}

template <typename T>
queuemt<T>::queuemt(size_t max_queue_size)
	: maxsize(max_queue_size)
{
}

template <typename T>
bool queuemt<T>::try_push(const T &new_value)
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	std::cout << "Begin push by ref - " << new_value << std::endl;
	if (maxsize == 0 || data_queue.size() < maxsize)
	{
		data_queue.push(new_value);
		std::cout << "End push by ref." << std::endl;
		return true;
	}
	else
	{
		std::cout << "Queue is full !" << std::endl;
		return false;
	}
}

template <typename T>
bool queuemt<T>::try_push(T &&new_value)
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	std::cout << "Begin push by rvalue - " << new_value << std::endl;
	if (maxsize == 0 || data_queue.size() < maxsize)
	{
		data_queue.push(std::move(new_value));
		std::cout << "End push by rvalue." << std::endl;
		return true;
	}
	else
	{
		std::cout << "Queue is full !" << std::endl;
		return false;
	}
}

// template <typename T>
// inline bool queuemt<T>::wait_and_push(const T &new_value) {
// 	return false;
// }

template <typename T>
bool queuemt<T>::try_pop(T &value)
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	if (!data_queue.empty())
	{
		value = std::move(data_queue.front());
		data_queue.pop();
		return true;
	}
	return false;
}

#endif

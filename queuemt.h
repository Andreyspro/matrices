//Based in "C++ Concurrency in Action  Anthony Williams"

#ifndef QUEUEMT_H
#define QUEUEMT_H
#define no_queue_mt_debug

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <atomic>

template <typename T>
class queuemt
{
private:
	std::queue<T> data_queue;
	mutable std::mutex queue_mutex;
	size_t maxsize;
	std::atomic<bool> end_of_service;
	std::condition_variable notify_push_cv;
	std::condition_variable notify_pop_cv;
public:
	queuemt();
	queuemt(size_t set_maxsize);
	bool try_push(const T &new_value);
	bool try_push(T &&new_value);
	bool wait_and_push(T &&value);
	bool try_pop(T &value);
	bool wait_and_pop(T &value);
	bool empty();
	size_t size();
	void free();
	void stop_service();
};

template <typename T>
queuemt<T>::queuemt()
	: maxsize(0)
{
	end_of_service.store(false);
}

template <typename T>
queuemt<T>::queuemt(size_t max_queue_size)
	: maxsize(max_queue_size)
{
	end_of_service.store(false);
}

template <typename T>
bool queuemt<T>::try_push(const T &new_value)
{
#ifdef _queue_mt_debug
	std::cout << "Queuemt. Begin push by ref - " << new_value << std::endl;
#endif
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	if (maxsize == 0 || data_queue.size() < maxsize)
	{
		data_queue.push(new_value);
		notify_push_cv.notify_one();
#ifdef _queue_mt_debug
		std::cout << "Queuemt. End push by ref. Size - " << data_queue.size() << "\n";
#endif
		return true;
	}
	else
	{
		// std::cout << "Queuemt. Queue is full !" << std::endl;
		return false;
	}
}

template <typename T>
bool queuemt<T>::try_push(T &&new_value)
{
#ifdef _queue_mt_debug
	std::cout << "Queuemt. Begin push by rvalue - " << new_value << std::endl;
#endif
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	if (maxsize == 0 || data_queue.size() < maxsize)
	{
		data_queue.push(std::move(new_value));
		notify_push_cv.notify_one();
#ifdef _queue_mt_debug
		std::cout << "Queuemt. End push by rvalue. Size - " << data_queue.size() << "\n";
#endif
		return true;
	} else
		return false;
}

template <typename T>
bool queuemt<T>::wait_and_push(T &&value)
{
#ifdef _queue_mt_debug
	std::cout << "Queuemt. Start wait_and_push. " << std::endl;
#endif
	std::unique_lock<std::mutex> lk(queue_mutex);
	notify_pop_cv.wait(lk, [this] {return (data_queue.size() < maxsize) || end_of_service.load();});
	if (!end_of_service.load())
	{
		data_queue.push(std::move(value));
		notify_push_cv.notify_one();
		return true;
	}
	return false;
}

template <typename T>
bool queuemt<T>::try_pop(T &value)
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	if (!data_queue.empty())
	{
		value = std::move(data_queue.front());
		data_queue.pop();
		notify_pop_cv.notify_one();
		return true;
	}
	return false;
}

template <typename T>
bool queuemt<T>::wait_and_pop(T &value)
{
#ifdef _queue_mt_debug
	std::cout << "Queuemt. Start wait_and_pop. " << std::endl;
#endif
	std::unique_lock<std::mutex> lk(queue_mutex);
	notify_push_cv.wait(lk, [this] {return !data_queue.empty() || end_of_service.load();});
	if (!data_queue.empty())
	{
		value = std::move(data_queue.front());
		data_queue.pop();
		notify_pop_cv.notify_one();
		return true;
	}
	return false;
}

template <typename T>
bool queuemt<T>::empty()
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	return data_queue.empty();
}

template <typename T>
size_t queuemt<T>::size()
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	return data_queue.size();
}

template<typename T>
void queuemt<T>::free()
{
	return;
}

template <typename T>
void queuemt<T>::stop_service()
{
	std::lock_guard<std::mutex> lk_guard(queue_mutex);
	end_of_service.store(true);
	notify_pop_cv.notify_all();
	notify_push_cv.notify_all();
}

#endif

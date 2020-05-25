#pragma once
#include <functional>
#include <future>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool
{
public:
	ThreadPool();
	~ThreadPool();

	template<class T, class... Args>
	auto Enqueue(T&& func, Args&&... args)
		->std::future<typename std::result_of<T(Args...)>::type>;

	const size_t GetTotalThreadCount() const;

private:
	size_t thread_count;
	std::vector<std::thread> threads;
	std::queue<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable condition;
	bool stop;
};

inline ThreadPool::ThreadPool()
	: stop(false)
{
	thread_count = std::thread::hardware_concurrency();
	threads.reserve(thread_count);

	for (int i = 0; i < thread_count; ++i)
	{
		threads.emplace_back(
			[this] 
			{
				while (true)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->mutex);
						this->condition.wait(lock, [this] {return this->stop || !this->tasks.empty(); });

						if (this->stop && this->tasks.empty())
							return;

						task = std::move(this->tasks.front());
						tasks.pop();
					}

					task();
				}
			}
		);
	}
}

inline ThreadPool::~ThreadPool()
{
	std::unique_lock<std::mutex> lock(mutex);
	stop = true;

	condition.notify_all();
	for (std::thread& thread : threads)
		thread.join();

	threads.clear();
}

inline const size_t ThreadPool::GetTotalThreadCount() const
{
	return thread_count;
}

template<class T, class ...Args>
inline auto ThreadPool::Enqueue(T&& func, Args&& ...args) -> std::future<typename std::result_of<T(Args ...)>::type>
{
	if (stop)
		throw std::runtime_error("You are trying to enqueue to stopped threadPool.");

	using return_type = typename std::result_of<T(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<T>(func), std::forward<Args>(args)...));

	std::future<return_type> result = task->get_future();
	{
		std::unique_lock<std::mutex> lock(mutex);

		tasks.emplace([task]() {(*task)(); });
	}

	condition.notify_one();

	return result;
}
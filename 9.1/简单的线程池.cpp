#include <thread>
#include <vector>
#include <atomic>
#include "thread_safe_queue.h"
#include "join_threads.h"

class thread_pool
{
	/*
	注意成员声明的顺序很重要：done标志和worker_queue必须在threads数组
	之前声明，而数据必须在joiner前声明。这就能确保成员能以正确的顺序
	销毁；比如，所有线程都停止运行时，队列就可以安全的销毁了。
	*/
	std::atomic_bool done;
	thread_safe_queue<std::function<void()>> work_queue;//1
	std::vector<std::thread> threads;//2
	join_threads joiner;//3

	void worker_thread()
	{
		while (!done)//4
		{
			std::function<void()> task;
			if (work_queue.try_pop(task))//5
				task();//6
			else
				std::this_thread::yield();//7
		}
	}
public:
	thread_pool() :done(false), joiner(threads)
	{
		unsigned const thread_count = std::thread::hardware_concurrency();//8
		try
		{
			for (unsigned i = 0; i < thread_count; ++i)
			{
				threads.push_back(
					std::thread(&thread_pool::worker_thread, this));//9
			}
		}
		catch (...)
		{
			done = true;//10
			throw;
		}
	}

	~thread_pool()
	{
		done = true;//11
	}

	template<typename FunctionType>
	void submit(FunctionType f)
	{
		work_queue.push(std::function<void()>(f));//12
	}
};
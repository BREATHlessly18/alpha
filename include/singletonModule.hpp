#pragma once

#include <memory>
#include <mutex>

template <typename T>
class SingletonModule
{
public:
	template <typename... Args>
	static T &getInstance(Args &&...args)
	{
		static std::once_flag flag;
		std::call_once(flag, [&]()
					   { mInstance.reset(new T(std::move<Args>(args)...)); });
		return *mInstance;
	}

private:
	SingletonModule() = default;
	SingletonModule(const SingletonModule &) = delete;
	SingletonModule &operator=(const SingletonModule &) = delete;
	~SingletonModule() = default;

private:
	static std::unique_ptr<T> mInstance;
};

template <typename T>
std::unique_ptr<T> SingletonModule<T>::mInstance;

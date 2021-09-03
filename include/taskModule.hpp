#pragma once

#include <condition_variable>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>

class TaskModule
{
public:
    enum class ThrState
    {
        eStoped,
        eRunning,
    };

    int start();
    int stop();
    bool isRunning();
    virtual ~TaskModule();

private:
    int run();

protected:
    TaskModule();
    TaskModule(const TaskModule &) = delete;
    TaskModule &operator=(const TaskModule &) = delete;
    TaskModule(const TaskModule &&) = delete;
    TaskModule &operator=(const TaskModule &&) = delete;
    const char *getStateDesc(ThrState state);
    TaskModule::ThrState getState();

    virtual int process() = 0;

private:
    ThrState mState;
    std::unique_ptr<std::thread> mThread;
    std::atomic<bool> mStartFlag;
};

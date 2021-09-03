#include "taskModule.hpp"
#include <stdio.h>

TaskModule::TaskModule()
    : mState{ThrState::eStoped}, mThread{nullptr}, mStartFlag{false}
{
    printf("\t ctor %s, %p\n", __func__, this);
}

TaskModule::~TaskModule()
{
    printf("\t dtor %s, %p\n", __func__, this);
    mStartFlag = false;
    mState = ThrState::eStoped;
}

int TaskModule::start()
{
    if (mStartFlag == true)
    {
        printf("thread is running!\n");
        return -1;
    }
    mThread.reset(std::move(new std::thread(&TaskModule::run, this)));
    mStartFlag = true;
    return 0;
}

int TaskModule::stop()
{
    if (!mThread->joinable())
    {
        return -1;
    }
    mStartFlag = false;
    mState = ThrState::eStoped;

    mThread->join();
    mThread.reset();

    return 0;
}

bool TaskModule::isRunning()
{
    return mStartFlag.load();
}

int TaskModule::run()
{
    while (1)
    {
        process();
    }

    return 0;
}

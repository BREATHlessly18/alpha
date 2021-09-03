#include "frameQueue.hpp"

FrameQueue::FrameQueue()
    : mStrQue{}, mMtx{}, mCv{}
{
    ::printf("ctor %s, %p\n", __func__, this);
}

FrameQueue::~FrameQueue()
{
    ::printf("dtor %s, %p\n", __func__, this);
}

int FrameQueue::insert(const std::string &str)
{
    std::unique_lock<std::mutex> lock(mMtx);
    if (mStrQue.size() >= 30)
    {
        lock.unlock();
        mCv.notify_all();
        return -1;
    }
    mStrQue.push_back(str);
    lock.unlock();
    mCv.notify_one();
    return 0;
}

int FrameQueue::pop(std::string &str)
{
    std::unique_lock<std::mutex> lock(mMtx);
    mCv.wait(lock, [this]
             { return (mStrQue.empty()) ? false : true; });
    str = mStrQue.front();
    mStrQue.pop_front();
    lock.unlock();
    mCv.notify_one();
    return 0;
}

size_t FrameQueue::size()
{
    std::unique_lock<std::mutex> lock(mMtx, std::try_to_lock);
    return mStrQue.size();
}

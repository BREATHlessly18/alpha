#pragma once

#include <condition_variable>
#include <string>
#include <memory>
#include <mutex>
#include <deque>

class FrameQueue
{
public:
    FrameQueue();
    ~FrameQueue();

    int insert(const std::string &str);
    int pop(std::string &str);
    size_t size();

private:
    std::deque<std::string> mStrQue;
    std::mutex mMtx;
    std::condition_variable mCv;
};

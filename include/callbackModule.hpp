#pragma once

#include "cameraCommon.hpp"
#include "frameQueue.hpp"

class VideoFrameProccess
    : public VideoFrameInterface
{
public:
    VideoFrameProccess(std::shared_ptr<FrameQueue> &que)
        : VideoFrameInterface(),
          mQue{que}
    {
    }

    ~VideoFrameProccess() = default;

    void SendFrame(const VideoBuffer &frame) override
    {
        mQue->insert(std::string((char *)frame.getBuf(), frame.getLength()));
    }

    void RecvFrame(std::string &frame) override
    {
        mQue->pop(frame);
    }

private:
    std::shared_ptr<FrameQueue> mQue;
};

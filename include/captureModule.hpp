#pragma once

#include <vector>
#include "cameraCommon.hpp"
#include "taskModule.hpp"
#include "operateInterface.hpp"
#include "callbackModule.hpp"
#include "task.hpp"

class CaptureModule : public OperateInterface,
                      public work::Task
{
public:
    int Operate() override;
    int Start() override;
    int Stop() override;

public:
    CaptureModule();
    CaptureModule(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que);
    ~CaptureModule() override;
    void set(const FrameInfo &frameCap, std::shared_ptr<FrameQueue> &que);

    void threadProcess() override;

private:
    int initV4l2();
    int applyMemory();
    int releaseMemory();

private:
    const uint8_t DEFAULT_BUFFER_COUNT = 4;
    int mDeviceId;
    FrameInfo mCapability;
    std::unique_ptr<VideoFrameInterface> mpFrameSnder;
    std::vector<VideoBuffer> mVideoFramePool;
};
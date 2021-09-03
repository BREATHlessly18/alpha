#pragma once

#include <memory>
#include "captureModule.hpp"
#include "codecModule.hpp"
#include "frameQueue.hpp"

class proccessModule
{
private:
    proccessModule(const FrameInfo &frame);
    ~proccessModule();

public:
    static auto getInstance() -> proccessModule const &;
    auto start() const -> void;
    auto stop() const -> void;

private:
    std::unique_ptr<CaptureModule> mCapture;
    std::unique_ptr<CodecModule> mCodec;
    std::shared_ptr<FrameQueue> mFrameQue;
};

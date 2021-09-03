#include "proccessModule.hpp"

proccessModule::proccessModule(const FrameInfo &frame)
    : mCapture{nullptr}, mCodec{nullptr}, mFrameQue{new FrameQueue}
{
    mCapture.reset(new CaptureModule(frame, mFrameQue));
    mCodec.reset(new CodecModule(frame, mFrameQue));
}

proccessModule::~proccessModule()
{
}

auto proccessModule::getInstance() -> proccessModule const &
{
    const FrameInfo frame{30, 0, 1080, 720};
    static proccessModule _instance(frame);
    return _instance;
}

auto proccessModule::start() const -> void
{
    mCapture->Start();
    mCodec->Start();
}

auto proccessModule::stop() const -> void
{
    mCodec->Stop();
    mCapture->Stop();
}

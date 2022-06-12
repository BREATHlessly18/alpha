#include "capture/worker.hpp"

#include "base/logger.hpp"

namespace video_capture {
namespace capture {

Worker::Worker(const FrameInfo &frame)
    : capture_{}, codec_{}, frame_que_{new FrameQueue} {
  capture_.reset(new CaptureModule(frame, frame_que_));
  codec_.reset(new CodecModule(frame, frame_que_));
}

Worker::~Worker() {}

auto Worker::getInstance() -> Worker const & {
  const FrameInfo frame{30, 0, 1080, 720};
  static Worker _instance(frame);
  return _instance;
}

auto Worker::start() const -> void {
  capture_->Start();
  codec_->Start();

  LOG_INFO("Start!");
}

auto Worker::stop() const -> void {
  codec_->Stop();
  capture_->Stop();

  LOG_INFO("Stop!");
}

}  // namespace capture
}  // namespace video_capture

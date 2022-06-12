#pragma once

#include <memory>

#include "capture/captureModule.hpp"
#include "capture/codecModule.hpp"
#include "capture/frameQueue.hpp"

namespace video_capture {
namespace capture {

class Worker {
 private:
  Worker(const FrameInfo &frame);
  ~Worker();

 public:
  static Worker const &getInstance();
  void start() const;
  void stop() const;

 private:
  std::unique_ptr<CaptureModule> capture_;
  std::unique_ptr<CodecModule> codec_;
  std::shared_ptr<FrameQueue> frame_que_;
};

}  // namespace capture
}  // namespace video_capture
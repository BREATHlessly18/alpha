#pragma once

#include "capture/cameraCommon.hpp"
#include "capture/frameQueue.hpp"

namespace video_capture {
namespace capture {

class VideoFrameProccess : public VideoFrameInterface {
 public:
  explicit VideoFrameProccess(std::weak_ptr<FrameQueue> que)
      : VideoFrameInterface(), que_{que} {}

  ~VideoFrameProccess() override = default;

  void SendFrame(const VideoBuffer &frame) override {
    auto q = que_.lock();
    if (q) {
      q->insert(std::string((char *)frame.buff(), frame.size()));
    }
  }

  void RecvFrame(std::string &frame) override {
    auto q = que_.lock();
    if (q) {
      q->pop(frame);
    }
  }

 private:
  std::weak_ptr<FrameQueue> que_;
};

}  // namespace capture
}  // namespace video_capture

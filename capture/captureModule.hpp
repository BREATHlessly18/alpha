#pragma once

#include <vector>

#include "base/task.hpp"
#include "base/taskModule.hpp"
#include "capture/callbackModule.hpp"
#include "capture/cameraCommon.hpp"
#include "capture/operateInterface.hpp"

namespace video_capture {
namespace capture {

class CaptureModule : public OperateInterface, public base::Task {
 public:
  int Operate() override;
  int Start() override;
  int Stop() override;

 public:
  CaptureModule();
  CaptureModule(const FrameInfo &frameCap, std::weak_ptr<FrameQueue> que);
  ~CaptureModule() override;
  void set(const FrameInfo &frameCap, std::weak_ptr<FrameQueue> que);

  void threadProcess() override;

 private:
  int initV4l2();
  int applyMemory();
  int releaseMemory();

 private:
  const uint8_t DEFAULT_BUFFER_COUNT = 4;
  int deviceId_;
  FrameInfo capability_;
  std::unique_ptr<VideoFrameInterface> frame_sender_;
  std::vector<VideoBuffer> video_frame_pool_;
};

}  // namespace capture
}  // namespace video_capture
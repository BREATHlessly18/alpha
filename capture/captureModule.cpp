#include "capture/captureModule.hpp"

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>

#include "base/logger.hpp"
#include "capture/callbackModule.hpp"

namespace video_capture {
namespace capture {

CaptureModule::CaptureModule()
    : OperateInterface(),
      Task(),
      deviceId_{-1},
      capability_{},
      frame_sender_{},
      video_frame_pool_{} {}

CaptureModule::CaptureModule(const FrameInfo &frameCap,
                             std::weak_ptr<FrameQueue> que)
    : OperateInterface(),
      Task(),
      deviceId_{-1},
      capability_{frameCap},
      frame_sender_{new VideoFrameProccess(que)},
      video_frame_pool_{} {}

CaptureModule::~CaptureModule() {}

void CaptureModule::set(const FrameInfo &frameCap,
                        std::weak_ptr<FrameQueue> que) {
  capability_ = frameCap;
  frame_sender_.reset(new VideoFrameProccess(que));
}

int CaptureModule::Start() {
  if (Task::running()) {
    printf("the capture task has already started\n");
    return 0;
  }

  if (initV4l2() == -1) {
    printf("initV4l2 fail\n");
    return -1;
  }

  if (applyMemory() == -1) {
    printf("applyMemory fail\n");
    return -1;
  }

  if (false == Task::start()) {
    printf("start video capture task fail\n");
    return -1;
  }

  v4l2_buf_type type{V4L2_BUF_TYPE_VIDEO_CAPTURE};
  if (-1 == ::ioctl(deviceId_, VIDIOC_STREAMON, &type)) {
    printf("turn on stream fail\n");
    return -1;
  }
  return 0;
}

int CaptureModule::Stop() {
  if (false == Task::stop()) {
    std::cerr << "stop capture task fail.\n";
    return -1;
  }

  if (-1 == releaseMemory()) {
    std::cerr << "releaseMemory fail.\n";
    return -1;
  }

  close(deviceId_);
  deviceId_ = -1;

  std::cout << "stop capture task success.\n";
  return 0;
}

int CaptureModule::initV4l2() {
  const char deviceName[]{"/dev/video0"};
  deviceId_ = open(deviceName, O_RDWR | O_NONBLOCK, 0);
  if (-1 == deviceId_) {
    printf("open %s with O_RDWR | O_NONBLOCK fail\n", deviceName);
    return -1;
  }

  v4l2_fmtdesc fmt{};
  fmt.index = 0;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  v4l2_format video_fmt{};
  video_fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  video_fmt.fmt.pix.sizeimage = 0;
  video_fmt.fmt.pix.width = capability_.width();
  video_fmt.fmt.pix.height = capability_.hight();
  video_fmt.fmt.pix.pixelformat = capability_.format();
  if (-1 == ::ioctl(deviceId_, VIDIOC_S_FMT, &video_fmt)) {
    std::cerr << "VIDIOC_S_FMT fail, erron = " << errno << std::endl;
    return -1;
  }
  v4l2_streamparm streamparms{};
  streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == ::ioctl(deviceId_, VIDIOC_G_PARM, &streamparms)) {
    std::cerr << "VIDIOC_G_PARM fail, errno = " << errno << std::endl;
    return -1;
  } else {
    ::memset(&streamparms, 0, sizeof(streamparms));
    streamparms.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    streamparms.parm.capture.timeperframe.numerator = 1;
    streamparms.parm.capture.timeperframe.denominator = 30;
    if (::ioctl(deviceId_, VIDIOC_S_PARM, &streamparms) < 0) {
      std::cerr << "VIDIOC_S_PARM fail, errno =" << errno << std::endl;
      return -1;
    }
  }

  return 0;
}

int CaptureModule::applyMemory() {
  if (-1 == deviceId_) {
    std::cerr << __func__ << ", parameters check fail\n";
    return -1;
  }

  v4l2_requestbuffers rbuffer{};
  rbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  rbuffer.memory = V4L2_MEMORY_MMAP;
  rbuffer.count = DEFAULT_BUFFER_COUNT;

  if (::ioctl(deviceId_, VIDIOC_REQBUFS, &rbuffer) < 0) {
    std::cerr << "VIDIOC_REQBUFS fail. errno = " << errno << std::endl;
    return -1;
  }

  // 申请buffer  入队等待
  video_frame_pool_.clear();
  for (uint8_t i{0}; i < rbuffer.count; ++i) {
    v4l2_buffer buffer{};
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buffer.memory = V4L2_MEMORY_MMAP;
    buffer.index = i;

    if (::ioctl(deviceId_, VIDIOC_QUERYBUF, &buffer) < 0) {
      std::cerr << "VIDIOC_QUERYBUF fail. errno =" << errno << std::endl;
      return -1;
    }

    VideoBuffer element(mmap(NULL, buffer.length, PROT_READ | PROT_WRITE,
                             MAP_SHARED, deviceId_, buffer.m.offset),
                        buffer.length);

    if (MAP_FAILED == element.buff()) {
      continue;
    }

    if (::ioctl(deviceId_, VIDIOC_QBUF, &buffer) < 0) {
      std::cerr << "VIDIOC_QBUF fail. errno =" << errno << std::endl;
      return -1;
    }
    video_frame_pool_.push_back(element);
  }

  LOG_INFO("fd:{}", deviceId_);
  return 0;
}

int CaptureModule::releaseMemory() {
  if (-1 == deviceId_) {
    std::cerr << __func__ << ", parameters check fail\n";
    return -1;
  }

  // 释放buffer
  for (uint8_t i{0}; i < video_frame_pool_.size(); ++i) {
    if (-1 ==
        munmap(video_frame_pool_[i].buff(), video_frame_pool_[i].size())) {
      std::cerr << "munmap buffer[" << i << "] fail\n";
    }
  }
  video_frame_pool_.clear();
  return 0;
}

void CaptureModule::threadProcess() {
  int ret{-1};
  fd_set rSet{};
  timeval timeout{};

  FD_ZERO(&rSet);
  FD_SET(deviceId_, &rSet);
  timeout.tv_sec = 3;
  timeout.tv_usec = 0;

  ret = select(deviceId_ + 1, &rSet, NULL, NULL, &timeout);
  if (ret < 0 and errno != EINTR) {
    return;
  } else if (0 == ret) {
    std::cerr << "time out\n";
    return;
  } else if (!FD_ISSET(deviceId_, &rSet)) {
    return;
  }

  // 从输出队列中取一帧
  v4l2_buffer buf{};
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  if (-1 == ::ioctl(deviceId_, VIDIOC_DQBUF, &buf)) {
    if (errno != EINTR) {
      std::cerr << "could not sync on a buffer on device\n"
                << strerror(errno) << "\n";
      return;
    }
  }

  if (buf.index >= DEFAULT_BUFFER_COUNT) {
    std::cerr << "buf.index invalid\n";
    return;
  }
  void *buffer = (uint8_t *)video_frame_pool_[buf.index].buff();
  uint32_t length = video_frame_pool_[buf.index].size();

  frame_sender_->SendFrame(VideoBuffer(buffer, length));

  // 回队
  if (-1 == ::ioctl(deviceId_, VIDIOC_QBUF, &buf)) {
    std::cerr << "failed to enqueue capture buffer\n";
    return;
  }
}

int CaptureModule::Operate() {
  threadProcess();
  return 0;
}

}  // namespace capture
}  // namespace video_capture

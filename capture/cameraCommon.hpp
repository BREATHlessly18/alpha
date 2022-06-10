#pragma once

#include <fcntl.h>            // for O_RDWR
#include <linux/videodev2.h>  // for v4l2 struct
#include <sys/ioctl.h>        // for ioctl()
#include <sys/mman.h>         // for mmap
#include <unistd.h>           // for open(), creat()

#include <cstdint>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/opt.h>
#include <libavutil/timestamp.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
// #include <libavcodec/internal.h>
}

struct VideoBuffer {
 public:
  VideoBuffer() : buff_{}, len_{0} {}
  VideoBuffer(void *buf, uint32_t len) : buff_{buf}, len_{len} {}

  void set_buff(void *buf) { buff_ = buf; }
  void set_size(uint32_t length) { len_ = length; }
  void *buff() const { return buff_; }
  uint32_t size() const { return len_; }

 private:
  void *buff_;
  uint32_t len_;
};

struct FrameInfo {
 public:
  FrameInfo() : fps_{0}, fmt_{-1}, width_{0}, hight_{0} {}
  explicit FrameInfo(int fps, int fmt, int w, int h)
      : fps_{fps}, fmt_{fmt}, width_{w}, hight_{h} {}

  int fps() const { return fps_; }
  int format() const { return fmt_; }
  int width() const { return width_; }
  int hight() const { return hight_; }
  void set_fps(const int fps) { fps_ = fps; }
  void set_format(const int fmt) { fmt_ = fmt; }
  void set_width(const int w) { width_ = w; }
  void set_hight(const int h) { hight_ = h; }

 private:
  int fps_;
  int fmt_;
  int width_;
  int hight_;
};

class VideoFrameInterface {
 public:
  virtual ~VideoFrameInterface() = default;
  virtual void SendFrame(const VideoBuffer &frame) {}
  virtual void RecvFrame(std::string &frame) {}
};

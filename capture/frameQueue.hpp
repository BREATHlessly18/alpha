#pragma once

#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

namespace video_capture {
namespace capture {

class FrameQueue {
 public:
  FrameQueue();
  ~FrameQueue();

  int insert(const std::string &str);
  int pop(std::string &str);
  size_t size();

 private:
  const uint8_t max_level = 30;
  std::deque<std::string> que_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

}  // namespace capture
}  // namespace video_capture

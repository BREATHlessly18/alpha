#include "capture/frameQueue.hpp"

namespace video_capture {
namespace capture {

FrameQueue::FrameQueue() : que_{}, mtx_{}, cond_{} {}

FrameQueue::~FrameQueue() {}

int FrameQueue::insert(const std::string &item) {
  std::unique_lock<std::mutex> lock(mtx_);

  if (que_.size() >= max_level) {
    lock.unlock();
    cond_.notify_all();
    return -1;
  }

  que_.emplace_back(std::move(item));
  lock.unlock();
  cond_.notify_one();
  return 0;
}

int FrameQueue::pop(std::string &item) {
  std::unique_lock<std::mutex> lock(mtx_);

  cond_.wait(lock, [this] { return not que_.empty(); });

  item = std::move(que_.front());
  que_.pop_front();

  lock.unlock();
  cond_.notify_one();
  return 0;
}

size_t FrameQueue::size() { return que_.size(); }

}  // namespace capture
}  // namespace video_capture

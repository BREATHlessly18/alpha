#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <thread>

namespace video_capture {
namespace base {

class Task {
  enum class State : uint8_t {
    kEnd,
    kPaused,
    kRunning,
  };

 public:
  Task();
  virtual ~Task();

  bool start();
  bool stop();
  bool pause();
  bool resume();

 protected:
  virtual void threadProcess() = 0;
  auto running() { return curState_ == State::kRunning; }

 private:
  void run();

 private:
  State curState_;
  std::atomic_bool pause_flag_;
  std::atomic_bool end_flag_;
  std::unique_ptr<std::thread> task_;
  std::mutex mtx_;
  std::condition_variable cond_;
};

}  // namespace base
}  // namespace video_capture

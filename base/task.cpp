#include "base/task.hpp"

#include <iostream>

#include "base/logger.hpp"

namespace video_capture {
namespace base {

Task::Task()
    : curState_{State::kEnd},
      pause_flag_{false},
      end_flag_{},
      task_{},
      mtx_{},
      cond_{} {}

Task::~Task() {}

bool Task::start() {
  if (curState_ == State::kRunning or nullptr != task_) {
    return false;
  }

  curState_ = State::kRunning;
  end_flag_.store(false);
  task_.reset(new std::thread(&Task::run, this));
  return true;
}

bool Task::stop() {
  curState_ = State::kEnd;
  end_flag_.store(true);

  if (task_ == nullptr) {
    return true;
  }

  if (!task_->joinable()) {
    LOG_ERROR("joinable : false");
    return false;
  }

  task_->join();
  task_.reset();
  cond_.notify_one();
  return true;
}

bool Task::pause() {
  if (curState_ == State::kRunning or end_flag_ == false or
      pause_flag_ == false or task_ != nullptr) {
    curState_ = State::kPaused;
    pause_flag_.store(true);
    return true;
  }

  LOG_ERROR("pause : false");
  return false;
}

bool Task::resume() {
  if (curState_ == State::kPaused or end_flag_ == false or
      pause_flag_ == true or task_ != nullptr) {
    curState_ = State::kRunning;
    pause_flag_.store(false);
    cond_.notify_one();
    return true;
  }

  LOG_ERROR("resume : false");
  return false;
}

auto Task::run() -> void {
  do {
    threadProcess();

    while (pause_flag_ == true) {
      std::unique_lock<std::mutex> locker(mtx_);
      cond_.wait(locker);
    }
  } while (end_flag_ == false);

  pause_flag_.store(false);
}

}  // namespace base
}  // namespace video_capture

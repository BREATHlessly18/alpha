#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace video_capture {
namespace base {

class TaskModule {
 public:
  enum class ThrState {
    eStoped,
    eRunning,
  };

  int start();
  int stop();
  bool running();
  virtual ~TaskModule();

 private:
  int run();

 protected:
  TaskModule();
  TaskModule(const TaskModule &) = delete;
  TaskModule &operator=(const TaskModule &) = delete;
  TaskModule(const TaskModule and) = delete;
  TaskModule &operator=(const TaskModule and) = delete;
  const char *getStateDesc(ThrState state);
  TaskModule::ThrState getState();

  virtual int process() = 0;

 private:
  ThrState mState;
  std::unique_ptr<std::thread> mThread;
  std::atomic<bool> mStartFlag;
};

}  // namespace base
}  // namespace video_capture
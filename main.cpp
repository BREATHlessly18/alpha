#include <signal.h>

#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>

#include "base/logger.hpp"
#include "capture/worker.hpp"
#include "nlohmann/json.hpp"

std::mutex mtx{};
std::condition_variable cond{};

using Worker = video_capture::capture::Worker;
using Logger = video_capture::base::Logger;

// 主函数须携带采集时间参数，参数为0表示一直采集，其余正整数表示采集时长
int main() {
  nlohmann::json config;
  std::ifstream("capture-config.json") >> config;

  // init log
  auto rval = Logger::init(config["log_config"],
                           config["touch_loginit_file"].get<bool>());
  if (Logger::LogError::kOK != rval and rval == Logger::LogError::kparseError) {
    return -1;
  }

  // start
  Worker::getInstance().start();

  std::unique_lock<std::mutex> lock(mtx);
  cond.wait_for(lock,
                std::chrono::seconds(config["capture_second"].get<uint8_t>()));

  // stop
  Worker::getInstance().stop();

  return 0;
}

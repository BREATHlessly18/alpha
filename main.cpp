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
bool stop{};

using Worker = video_capture::capture::Worker;
using Logger = video_capture::base::Logger;

auto sigOperate(int sig) {
  Worker::getInstance().stop();

  stop = true;
  cond.notify_one();
}

// 主函数须携带采集时间参数，参数为0表示一直采集，其余正整数表示采集时长
int main() {
  nlohmann::json config;
  std::ifstream("capture-config.json") >> config;
  auto duration = config["capture_second"].get<uint8_t>();

  auto rval = Logger::init(config["log_config"],
                           config["touch_loginit_file"].get<bool>());
  if (Logger::LogError::kOK != rval and rval == Logger::LogError::kparseError) {
    return -1;
  }

  auto b = std::chrono::steady_clock::now();

  Worker::getInstance().start();

  std::unique_lock<std::mutex> lock(mtx);
  cond.wait_for(lock, std::chrono::seconds(duration));
  Worker::getInstance().stop();

  auto e = std::chrono::steady_clock::now();
  LOG_INFO("Spend:{}s",
           std::chrono::duration_cast<std::chrono::seconds>(e - b).count());

  return 0;
}

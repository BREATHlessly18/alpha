#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "base/logger.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

#pragma warning(disable : 4996)

namespace video_capture {
namespace base {

static std::map<spdlog::level::level_enum, std::string> klevel_to_string{
    {spdlog::level::trace, "trace"}, {spdlog::level::debug, "debug"},
    {spdlog::level::info, "info"},   {spdlog::level::warn, "warn"},
    {spdlog::level::err, "err"},     {spdlog::level::critical, "critical"}};

static auto LevelToString(spdlog::level::level_enum level) {
  return klevel_to_string[level];
}

static auto StringToLevel(std::string const& level_name) {
  for (auto& item : klevel_to_string) {
    if (0 == level_name.compare(item.second)) {
      return item.first;
    }
  }

  return spdlog::level::off;
}

Logger::LogConfig Logger::config_{};
bool Logger::init_{false};
std::ofstream Logger::ofs_{};

/**
 * @brief 打印当前时间
 */
static auto log_now() {
  auto t = std::chrono::system_clock::now();
  std::time_t c = std::chrono::system_clock::to_time_t(t);
  return std::put_time(std::localtime(&c), "%F %T");
}

Logger::LogError Logger::init(const nlohmann::json& config,
                              bool touch_loginit_file) {
  init_ = true;

  if (touch_loginit_file) {
    ofs_.open("LogInit.txt", std::ios_base::app);
    ofs_ << "parse json config start. " << log_now() << std::endl;
  }

  return parse_config(config);
}

void Logger::flushLoginit() {
  if (ofs_.is_open()) {
    ofs_ << config_.dump() << std::endl;
    ofs_ << "parse json config over. " << log_now() << std::endl;
    ofs_.close();
  }
}

const Logger::LogError Logger::parse_config(const nlohmann::json& config) {
  if (not config.is_structured()) {
    return Logger::LogError::kconfigWrong;
  }

  try {
    std::string str{};
    std::stringstream ss;

    // log_path
    config.at("log_path").get_to(config_.logpath_);

    // max_log_size
    config.at("max_log_size").get_to(str);

    int base{1};
    for (size_t i = 0; i < str.size(); i++) {
      if (!isdigit(str[i])) {
        if (str[i] == 'M' or str[i] == 'm') {
          base = 1024 * 1024;
        } else if (str[i] == 'K' or str[i] == 'k') {
          base = 1024;
        } else {
          return Logger::LogError::kparseError;
        }

        ss << std::string(str.data(), i);
        ss >> config_.maxLogSize_;
        ss.clear();
        config_.maxLogSize_ *= base;
        break;
      }
    }

    // max_rotate_file_number
    config.at("max_rotate_file_number").get_to(config_.maxRotateFileNum_);

    // log_level
    str.clear();
    config.at("log_level").get_to(str);
    config_.loglvl_ = StringToLevel(str);

    // is_console
    config_.console_ = config.at("is_console").get<bool>();

  } catch (const spdlog::spdlog_ex& e) {
    return Logger::LogError::kparseError;
  }

  flushLoginit();
  return Logger::LogError::kOK;
}

Logger& Logger::instance() {
  static Logger instance;
  return instance;
}

Logger::Logger() : logger_{} {
  if (init_) {
    if (!config_.console_) {
      // 落盘为回滚日志
      logger_ = spdlog::rotating_logger_mt(
          "submaster", config_.logpath_, config_.maxLogSize_,
          config_.maxRotateFileNum_, false, {});
    } else {
      // 输出到控制台
      logger_ = spdlog::stdout_color_st("submaster_console");
    }

    spdlog::set_default_logger(logger_);
    logger_->set_level(config_.loglvl_);
  }
}

Logger::~Logger() {
  if (logger_) {
    spdlog::drop_all();
  }
}

}  // namespace base
}  // namespace video_capture

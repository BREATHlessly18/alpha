/**
 * @file logger.hh
 * @author yangzheng (yangzheng@any3.com)
 * @brief Spd log封装类，对外提供日志服务
 * @version 0.1
 * @date 2022-05-08
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include <fstream>
#include <memory>
#include <string>

#include "nlohmann/json.hpp"  // nlohmann json
#include "spdlog/spdlog.h"    // spdlog

namespace video_capture {
namespace base {

class Logger {
 public:
  enum class LogError : int8_t {
    kparseError = -1,
    kOK,
    kconfigWrong,
    kinitFileError
  };

 public:
  /**
   * @brief 初始化日志系统，在创建Logger前调用
   *
   * @param config json类型配置对象
   * @return errono 参考 LogError
   */
  static LogError init(const nlohmann::json& config = {},
                       bool touch_loginit_file = false);

  /**
   * @brief 获取Logger对象单例
   *
   * @return Logger& 实例
   */
  static Logger& instance();

  /**
   * @brief 获取spdlog对象，引用计数改变
   *
   * @return std::shared_ptr<spdlog::logger> logger对象引用[指针]
   */
  auto spd_logger() { return logger_; }

 private:
  /**
   * @brief 构造器 允许拷贝和移动，拷贝移动遵从默认规则
   *
   */
  Logger();

  /**
   * @brief 析构器 析构时回释放spdlog日志系统中的sink
   *
   */
  ~Logger();

  /**
   * @brief 私有 记录日志初始化数据
   *
   */
  static void flushLoginit();

  /**
   * @brief 私有：解析配置
   *
   * @param config json类型配置对象
   * @return errono 参考 LogError
   */
  static const LogError parse_config(const nlohmann::json& config = {});

  /**
   * @brief 内嵌结构体，记录配置信息
   *
   */
  struct LogConfig {
    bool console_ = true;
    spdlog::level::level_enum loglvl_ = spdlog::level::trace;
    size_t maxLogSize_ = 0;
    size_t maxRotateFileNum_ = 0;
    std::string logpath_ = {};

    std::string dump() {
      std::string os{};
      os.append("console_:")
          .append(std::to_string(console_))
          .append(",")
          .append("loglvl_:")
          .append(std::to_string(loglvl_))
          .append(",")
          .append("maxLogSize_:")
          .append(std::to_string(maxLogSize_))
          .append(",")
          .append("maxRotateFileNum_:")
          .append(std::to_string(maxRotateFileNum_))
          .append(",")
          .append("logpath_:")
          .append(logpath_);

      return os;
    }
  };

 private:
  static LogConfig config_;  ///< 配置文件对象
  static bool init_;         ///< 是否已经初始化
  static std::ofstream ofs_;  ///< 在日志系统初始化前记录配置读取数据
  std::shared_ptr<spdlog::logger> logger_;  ///< spdlog对象
};

#define LOG_TRACE(...)                                                     \
  SPDLOG_LOGGER_CALL(video_capture::base::Logger::instance().spd_logger(), \
                     spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...)                                                     \
  SPDLOG_LOGGER_CALL(video_capture::base::Logger::instance().spd_logger(), \
                     spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...)                                                      \
  SPDLOG_LOGGER_CALL(video_capture::base::Logger::instance().spd_logger(), \
                     spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...)                                                      \
  SPDLOG_LOGGER_CALL(video_capture::base::Logger::instance().spd_logger(), \
                     spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...)                                                     \
  SPDLOG_LOGGER_CALL(video_capture::base::Logger::instance().spd_logger(), \
                     spdlog::level::err, __VA_ARGS__)

}  // namespace base
}  // namespace video_capture

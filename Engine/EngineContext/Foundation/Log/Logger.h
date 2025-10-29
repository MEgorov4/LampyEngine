#pragma once
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "LogVerbosity.h"
namespace EngineCore::Foundation {
class ILogSink {
public:
  virtual ~ILogSink() = default;
  virtual void write(LogVerbosity level, const std::string &category,
                     const std::string &message) = 0;
};

class ConsoleSink final : public ILogSink {
public:
  void write(LogVerbosity level, const std::string &category,
             const std::string &message) override;
};

class LTLogger {
public:
  static LTLogger &Instance();

  void addSink(std::shared_ptr<ILogSink> sink);
  void clearSinks();

  void log(LogVerbosity level, const std::string &category,
           const std::string &message);
  void info(const std::string &cat, const std::string &msg);
  void warn(const std::string &cat, const std::string &msg);
  void error(const std::string &cat, const std::string &msg);
  void debug(const std::string &cat, const std::string &msg);

private:
  LTLogger() = default;

  std::vector<std::shared_ptr<ILogSink>> m_sinks;
  std::mutex m_mutex;
};
} // namespace EngineCore::Foundation

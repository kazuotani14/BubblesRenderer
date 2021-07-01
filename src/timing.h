#pragma once

#include <chrono>
#include <string>

namespace timing
{
  static std::chrono::time_point<std::chrono::system_clock> t_start;

  inline void tic()
  {
    t_start = std::chrono::system_clock::now();
  }

  inline void toc(const std::string &label = "")
  {
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - t_start).count();
    std::cerr << "[" << label << "] seconds elapsed: " << elapsed * 1e-6 << std::endl;
  }
}

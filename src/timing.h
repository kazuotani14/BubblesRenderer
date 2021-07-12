#pragma once

#include <chrono>
#include <iostream>
#include <string>
#include <unordered_map>

namespace timing
{
  // Simple tic/toc utility. Uses global var so only supports timing of one thing at a time within scope
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

  // Global timing tracker
  // Based on https://github.com/ethz-asl/schweizer_messer/blob/master/sm_timing/src/Timer.cpp
  struct TimingData
  {
    double total_time;
    double num;
  };

  inline std::unordered_map<std::string, TimingData> &data_instance()
  {
    static std::unordered_map<std::string, TimingData> data;
    return data;
  }

  inline void print(std::ostream &out)
  {
    out << "Timing data:" << std::endl;

    for (const auto &kv_pair : data_instance())
    {
      const auto &tag = kv_pair.first;
      const auto &data = kv_pair.second;

      out << tag << ": " << data.total_time << " | " << data.num << " | " << data.total_time / data.num << std::endl;
    }
  }

  class Timer
  {
  public:
    Timer(const std::string &tag)
        : tag_(tag), t_start_(std::chrono::system_clock::now()), active_(true)
    {
    }

    ~Timer()
    {
      if (active_)
        stop();
    }

    void stop()
    {
      auto now = std::chrono::system_clock::now();
      double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - t_start_).count() * 1e-6;

      data_instance()[tag_].total_time += elapsed;
      data_instance()[tag_].num++;

      active_ = false;
    }

  private:
    std::string tag_;
    std::chrono::time_point<std::chrono::system_clock> t_start_;
    bool active_;
  };
}

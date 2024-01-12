#include <chrono>

// Adapted into a class from the code example here: https://en.cppreference.com/w/cpp/chrono/steady_clock/now

class Timer
{
  std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration> start_time;

public:
  void Start()
  {
    start_time = std::chrono::steady_clock::now();
  }

  double Stop()
  {
    auto end_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end_time - start_time;
    return diff.count();
  }
};
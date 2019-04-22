#include "monotonicSeconds.hpp"
#include <chrono>

double monotonicSeconds()
{
    const auto now = std::chrono::steady_clock::now();
    const auto usec = std::chrono::time_point_cast<std::chrono::microseconds>(now);
    return usec.time_since_epoch().count() / 1'000'000.0;
}

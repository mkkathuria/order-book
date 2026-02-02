#include "rate_limiter.h"

RateLimiter::RateLimiter(std::chrono::steady_clock::duration interval)
    : interval_(interval) {}

bool RateLimiter::try_acquire() {
    std::lock_guard<std::mutex> lock(mu_);
    auto now = std::chrono::steady_clock::now();
    if (called_ && (now - last_) < interval_)
        return false;
    last_   = now;
    called_ = true;
    return true;
}

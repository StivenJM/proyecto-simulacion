#pragma once

#include <fstream>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace core {

inline void logCoreTiming(const std::string& message)
{
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    std::cerr << message;
    if (message.empty() || message.back() != '\n') {
        std::cerr << '\n';
    }

    std::ofstream file("core_timing.log", std::ios::app);
    if (file) {
        file << message;
        if (message.empty() || message.back() != '\n') {
            file << '\n';
        }
    }

#ifdef _WIN32
    std::string debugMessage = message;
    if (debugMessage.empty() || debugMessage.back() != '\n') {
        debugMessage.push_back('\n');
    }
    OutputDebugStringA(debugMessage.c_str());
#endif
}

class CoreTimingLine {
public:
    CoreTimingLine() = default;
    CoreTimingLine(const CoreTimingLine&) = delete;
    CoreTimingLine& operator=(const CoreTimingLine&) = delete;

    CoreTimingLine(CoreTimingLine&& other) noexcept
        : stream_(std::move(other.stream_)), active_(other.active_)
    {
        other.active_ = false;
    }

    ~CoreTimingLine()
    {
        if (active_) {
            logCoreTiming(stream_.str());
        }
    }

    template <typename T>
    CoreTimingLine& operator<<(const T& value)
    {
        stream_ << value;
        return *this;
    }

private:
    std::ostringstream stream_;
    bool active_ = true;
};

inline CoreTimingLine coreTiming()
{
    return CoreTimingLine{};
}

} // namespace core

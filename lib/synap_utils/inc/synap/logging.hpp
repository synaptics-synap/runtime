// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap logging utilities
///

#pragma once

#include <iostream>
#include <ostream>
#include <sstream>

#ifdef __ANDROID__
#include <android/log.h>
#endif

#define LOGV SYNAP_LOG(0)
#define LOGI SYNAP_LOG(1)
#define LOGW SYNAP_LOG(2)
#define LOGE SYNAP_LOG(3)

#define SYNAP_LOG(level)                                                              \
    synaptics::synap::Logger::enabled(level) && synaptics::synap::Logger{level}.out() \
                                                    << __FUNCTION__ << "():" << __LINE__ << ": "

namespace synaptics {
namespace synap {

class Logger {
public:
    Logger(int level) : _level(level) {}
    std::ostream& out() { return _log_message; }

    ~Logger()
    {
        static const char module[] = "SyNAP";
        static const char level_msg[] = "VIWE????";
        const std::string& log_string = _log_message.str();
#if defined(SYNAP_LOGS_TO_STDERR) || !defined(__ANDROID__)
        std::cerr << level_msg[_level] << ':' << module << ": " << log_string << std::endl;
#endif
#ifdef __ANDROID__
        __android_log_print(to_android_priority(_level), module, "%s", log_string.c_str());
#endif
    }

    /// @return true if the specified log level is enabled
    static bool enabled(int level)
    {
        static char* envs = getenv("SYNAP_NB_LOG_LEVEL");
        if (!envs)
            return level >= 2;
        static int env_level = atoi(envs);
        return env_level >= 0 && level >= env_level;
    }


private:
    int _level;
    std::ostringstream _log_message;
#ifdef __ANDROID__
    static int to_android_priority(int level)
    {
        switch (level) {
        case 0:
            return ANDROID_LOG_VERBOSE;
        case 1:
            return ANDROID_LOG_INFO;
        case 2:
            return ANDROID_LOG_WARN;
        case 3:
            return ANDROID_LOG_ERROR;
        default:
            return ANDROID_LOG_UNKNOWN;
        }
    }
#endif
};


}  // namespace synap
}  // namespace synaptics

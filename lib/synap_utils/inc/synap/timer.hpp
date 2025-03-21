/*
 * NDA AND NEED-TO-KNOW REQUIRED
 *
 * Copyright (C) 2013-2020 Synaptics Incorporated. All rights reserved.
 *
 * This file contains information that is proprietary to Synaptics
 * Incorporated ("Synaptics"). The holder of this file shall treat all
 * information contained herein as confidential, shall use the
 * information only for its intended purpose, and shall not duplicate,
 * disclose, or disseminate any of this information in any manner
 * unless Synaptics has otherwise provided express, written
 * permission.
 *
 * Use of the materials may require a license of intellectual property
 * from a third party or from Synaptics. This file conveys no express
 * or implied licenses to any intellectual property rights belonging
 * to Synaptics.
 *
 * INFORMATION CONTAINED IN THIS DOCUMENT IS PROVIDED "AS-IS", AND
 * SYNAPTICS EXPRESSLY DISCLAIMS ALL EXPRESS AND IMPLIED WARRANTIES,
 * INCLUDING ANY IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE, AND ANY WARRANTIES OF NON-INFRINGEMENT OF ANY
 * INTELLECTUAL PROPERTY RIGHTS. IN NO EVENT SHALL SYNAPTICS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, PUNITIVE, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF OR IN CONNECTION WITH THE USE
 * OF THE INFORMATION CONTAINED IN THIS DOCUMENT, HOWEVER CAUSED AND
 * BASED ON ANY THEORY OF LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, AND EVEN IF SYNAPTICS WAS
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. IF A TRIBUNAL OF
 * COMPETENT JURISDICTION DOES NOT PERMIT THE DISCLAIMER OF DIRECT
 * DAMAGES OR ANY OTHER DAMAGES, SYNAPTICS' TOTAL CUMULATIVE LIABILITY
 * TO ANY PARTY SHALL NOT EXCEED ONE HUNDRED U.S. DOLLARS.
 */
///
/// Duration measurement.
///

#pragma once

#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>

namespace synaptics {
namespace synap {

/// Measure time duration.
/// Timer is automatically started when object is created unless explicitely disabled.
class Timer {
    typedef std::chrono::steady_clock Clock;

public:
    typedef int64_t Duration;
    typedef std::chrono::microseconds DurationUnit;

    /// Create timer and start time measurement.
    Timer(bool auto_start = true)
    {
        if (auto_start)
            start();
    }

    /// Start time measurement.
    /// A timer can be (re)started multiple times if needed.
    void start() { _start = Clock::now(); }

    /// Get elapsed time since last start.
    /// @return duration in microseconds
    Duration get() const
    {
        auto end = Clock::now();
        return std::chrono::duration_cast<DurationUnit>(end - _start).count();
    }

private:
    Clock::time_point _start;
};


/// Print timer in ms
inline std::ostream& operator<<(std::ostream& out, const Timer& tmr)
{
    float t = tmr.get() / 1.e3;
    out << std::fixed << std::setprecision(2) << t << " ms";
    return out;
}

}  // namespace synap
}  // namespace synaptics

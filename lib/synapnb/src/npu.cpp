// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


#include "synap/npu.hpp"
#include "synap/logging.hpp"
#include "synap_device.h"
#include "synap_driver.hpp"

using namespace std;

namespace synaptics {
namespace synap {


/// Npu private implementation.
struct Npu::Private {
    bool _available{};
    bool _locked{};
};


Npu::Npu() : d{new Npu::Private}
{
    d->_available = synap_driver_available();
}


Npu::~Npu()
{
    // Release inference lock(s) if we had any
    unlock();
}


bool Npu::available() const
{
    return d->_available;
}


bool Npu::lock()
{
    if (d->_available && !d->_locked && synap_lock_npu()) {
        LOGV << "NPU Locked";
        d->_locked = true;
    }
    return d->_locked;
}


bool Npu::unlock()
{
    if (d->_locked) {
        LOGV << "Unlocking NPU";
        if (!synap_unlock_npu()) {
            return false;
        }
        d->_locked = false;
    }
    return true;
}


bool Npu::is_locked() const
{
    return d->_locked;
}


}  // namespace synap
}  // namespace synaptics

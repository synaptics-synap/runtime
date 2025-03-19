// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.


#include "synap/logging.hpp"
#include "synap_device.h"
#include "synap_driver.hpp"

using namespace std;

namespace synaptics {
namespace synap {


class SynapDriver {
public:
    SynapDriver() : _initialized{synap_init()}
    {
        LOGV << "Synap device initialized: " << _initialized;
    }

    ~SynapDriver()
    {
        if (_initialized) {
            LOGV << "Deinitializing Synap device";
            if (!synap_deinit()) {
                LOGE << "Failed to deinit Synap device";
            }
        }
    }

    bool available() const { return _initialized; }

private:
    bool _initialized{};
};


/// Helper function used to initalize synap device driver and close it at termination
bool synap_driver_available()
{
    static SynapDriver synap_driver;
    return synap_driver.available();
}


}  // namespace synap
}  // namespace synaptics

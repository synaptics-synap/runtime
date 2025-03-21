// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap NPU control class.
///

#pragma once
#include "synap/types.hpp"
#include <memory>

namespace synaptics {
namespace synap {


/// Reserve NPU usage.
class Npu {
public:
    Npu();
    ~Npu();

    /// @return true if NPU successfully initialized.
    bool available() const;

    /// Lock exclusive right to perform inference for the current process.
    /// All other processes attemping to execute inference will fail, including those using NNAPI.
    /// The lock will stay active until unlock() is called or the Npu object is deleted.
    /// @return true if NPU successfully locked,
    /// false if NPU unavailable or locked by another process.
    /// Calling this method on an Npu object that is already locked has no effect, just returns true
    bool lock();

    /// Release exclusive right to perform inference
    /// @return true if success.
    /// Calling this method on an Npu object that is not locked has no effect, just returns true
    bool unlock();

    /// @return true if we currently own the NPU lock.
    ///
    /// Note: the only way to test if the NPU is locked by someone else is to try to lock() it.
    bool is_locked() const;


private:
    // Implementation details
    struct Private;
    std::unique_ptr<Private> d;
};



}  // namespace synap
}  // namespace synaptics

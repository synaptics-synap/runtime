// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Synap command line initialization application.
/// 

#include <iostream>
#include <future>

#include "synap/arg_parser.hpp"
#include "synap/npu.hpp"

using namespace std;
using namespace synaptics::synap;

int main(int argc, char** argv)
{
    ArgParser args(argc, argv, "Synap NPU driver initialization", "[options]");
    bool interactive = args.has("-i", "Interactive mode, wait for user input to terminate");
    bool lock = args.has("--lock", "Keep NPU locked");
    args.check_help("--help", "Show help");
 
    Npu npu;
    if (!npu.available()) {
        cerr << "Error, NPU unavailable" << endl;
        return 1;
    }
    if (lock) {
        if (!npu.lock()) {
            cerr << "Error, NPU lock failed" << endl;
            return 1;
        }
        cout << "NPU locked" << endl;
    }

    if (interactive) {
        cout << "NPU ready, press enter to exit." << endl;
        cin.get();
        return 0;
    }

    // Wait forever
    cout << "NPU ready." << endl;
    std::promise<void>().get_future().wait();
    
    return 0;
}

// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: Copyright © 2019-2025 Synaptics Incorporated.
///
/// Image processing test application
///

#include "synap/allocator.hpp"
#include "synap/arg_parser.hpp"
#include "synap/buffer.hpp"
#include "synap/file_utils.hpp"
#include "synap/input_data.hpp"
#include "synap/preprocessor.hpp"
#include "synap/network.hpp"
#include "synap/image_postprocessor.hpp"
#include "synap/timer.hpp"

#ifdef ENABLE_PRIVATE_ALLOCATORS
#include "synap_allocators.hpp"
#endif
#include <iostream>
#include <string>

using namespace synaptics::synap;
using namespace std;


int main(int argc, char** argv)
{
    ArgParser args(argc, argv, "Image processing sample program", "[options] image-file(s)");
    string model = args.get("-m", "<file> Model file (.synap or legacy .nb)");
    string nb = args.get("--nb", "<file> Deprecated, same as -m");
    string meta = args.get("--meta", "<file> json meta file for legacy .nb models");
    string roi_str = args.get("--roi", "[x,y,h,w] Region of Interest");
    bool dump_raw = args.has("--dump-raw", "Save raw outputs to file");
    string out_dir = args.get("--out-dir", "<dir> Output directory");
#ifdef ENABLE_PRIVATE_ALLOCATORS
    bool use_cma = args.has("--cma", "Use contiguous memory allocation");
#endif

    args.check_help("--help", "Show help");
    validate_model_arg(model, nb, meta);

    // Select allocator for the buffers
#ifdef ENABLE_PRIVATE_ALLOCATORS
    Allocator* allocator = use_cma ? cma_allocator() : std_allocator();
#else
    Allocator* allocator = std_allocator();
#endif
    if (!allocator) {
        cerr << "Error, selected memory allocation method not available." << endl;
        return 1;
    }
    Rect roi{};
    if (!roi_str.empty() && !from_string(roi, roi_str)) {
        cerr << "Error, invalid ROI rectangle: " << roi_str << endl;
        return 1;
    }

    // Create and load the model
    Preprocessor preprocessor;
    Network net;
    ImagePostprocessor postprocessor;
    cout << "Loading network: " << model << endl;
    if (!net.load_model(model, meta)) {
        cerr << "Failed to load network" << endl;
        return 1;
    }

    // Set input tensors allocator
    for (Tensor& tensor : net.inputs) {
        cout << "Input buffer: " << tensor.name() << " size: " << tensor.size() << endl;
        tensor.buffer()->set_allocator(allocator);
    }

    // Set output tensors allocator
    for (Tensor& tensor : net.outputs) {
        cout << "Output buffer: " << tensor.name() << " size: " << tensor.size() << endl;
        tensor.buffer()->set_allocator(allocator);
    }

    // Run inference on all input images
    preprocessor.set_roi(roi);
    string input_filename;
    size_t file_index = 0;
    while ((input_filename = args.get()) != "") {
        cout << " " << endl;
        cout << "Input image: " << input_filename << endl;
        InputData image(input_filename);
        if (image.empty()) {
            cout << "Error, unable to read data from file: " << input_filename << endl;
            return 1;
        }
        if (!preprocessor.assign(net.inputs, image)) {
            cerr << "Failed to assign data to input tensors" << endl;
            return 1;
        }
        Timer t;
        bool success = net.predict();
        if (!success) {
            cout << "Predict failed" << endl;
            return 1;
        }
        cout << "Inference time: " << t << endl;

        if (!out_dir.empty() && out_dir[out_dir.size() - 1] != '/') {
            out_dir += "/";
        }
        string out_base = out_dir + "outimage" + to_string(file_index) + '_';

        if (dump_raw) {
            // Dump raw outputs
            int index = 0;
            for (const auto& out : net.outputs) {
                string filename = out_dir + "raw" + to_string(index++) + ".dat";
                cout << "Writing raw output: " << out.name() << " to file: " << filename << endl;
                binary_file_write(filename, out.data(), out.size());
            }
        }

        // Convert to nv12
        ImagePostprocessor::Result out_image = postprocessor.process(net.outputs);
        if (out_image.data.empty()) {
            cout << "Error, unable to convert network output to image" << endl;
            return 1;
        }
        string out_file = out_base + to_string(out_image.dim.x) + 'x' + to_string(out_image.dim.y) + '.' + out_image.ext;
        cout << "Writing output to file: " << out_file << endl;
        if (!binary_file_write(out_file, out_image.data.data(), out_image.data.size())) {
            cerr << "Failed to write output file" << endl;
        }
        file_index++;
    }

    return 0;
}

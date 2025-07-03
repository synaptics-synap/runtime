#define _POSIX_C_SOURCE 199309L

#include "synap_cnetwork.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>


/// generate dummy input (zero, constant, or random)
void fill_input_uint8(uint8_t* buffer, size_t len, const char* mode) {
    if (strcmp(mode, "zero") == 0) {
        memset(buffer, 0, len);
    } else if (strncmp(mode, "value:", 6) == 0) {
        int val = atoi(mode + 6);
        for (size_t i = 0; i < len; i++) {
            buffer[i] = (uint8_t)val;
        }
    } else if (strcmp(mode, "random") == 0) {
        for (size_t i = 0; i < len; i++) {
            buffer[i] = rand() % 256;
        }
    } else {
        fprintf(stderr, "Unknown input mode: %s\n", mode);
        exit(1);
    }
}

void cleanup(CNetwork* network,
             uint8_t* input_buffers[], int n_inputs,
             CNetworkInput* inputs,
             CNetworkOutput* outputs) {
    if (network) network_destroy(network);
    if (input_buffers)
        for (int i = 0; i < n_inputs; i++) {
            if (input_buffers[i]) {
                free(input_buffers[i]);
            }
        }
    free(input_buffers);
    free(inputs);
    free(outputs);
}

bool binary_file_write(const char *file_name, const void *data, size_t size) {
    FILE *fp = fopen(file_name, "wb");
    if (!fp) {
        fprintf(stderr, "Can't open file for writing: %s\n", file_name);
        return false;
    }

    if (data && size > 0) {
        size_t written = fwrite(data, 1, size, fp);
        if (written != size) {
            fprintf(stderr,
                    "Write error (wrote %zu of %zu bytes) to %s\n",
                    written, size, file_name);
            fclose(fp);
            return false;
        }
    }

    if (fclose(fp) != 0) {
        fprintf(stderr, "Can't close file: %s\n", file_name);
        return false;
    }

    return true;
}

int main(int argc, char** argv) {
    if (argc < 4) {
        printf("Usage: %s <model_file: .synap> <input_mode: zero | random | value:n> <repeat:n> [--dump-out] [--dump-raw]\n", argv[0]);
        return 1;
    }

    const char* model_path = argv[1];
    const char* input_mode = argv[2];
    int repeat = atoi(argv[3]);
    if (repeat < 1) {
        fprintf(stderr, "Invalid repeat count: %s\n", argv[3]);
        return 1;
    }

    bool dump_out = false;
    bool dump_raw = false;
    for (int i = 4; i < argc; i++) {
        if (strcmp(argv[i], "--dump-out") == 0) {
            dump_out = true;
        } else if (strcmp(argv[i], "--dump-raw") == 0) {
            dump_raw = true;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return 1;
        }
    }
    srand((unsigned int)time(NULL));
    struct timespec start, end;
    long infer_times[3] = {};

    CNetwork* network = network_create();
    if (!network) {
        fprintf(stderr, "Failed to create network\n");
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    if (!network_load(network, model_path)) {
        fprintf(stderr, "Failed to load model: %s\n", model_path);
        network_destroy(network);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    infer_times[0] = end.tv_nsec - start.tv_nsec;
    printf("Loaded network: %s\n\n", model_path);
    size_t n_inputs = network_get_tensor_count(network, TENSOR_TYPE_INPUT);
    size_t n_outputs = network_get_tensor_count(network, TENSOR_TYPE_OUTPUT);

    uint8_t** input_buffers = calloc(n_inputs, sizeof(uint8_t*));
    CNetworkInput* inputs = calloc(n_inputs, sizeof(CNetworkInput));
    CNetworkOutput* outputs = calloc(n_outputs, sizeof(CNetworkOutput));
    if (!input_buffers || !inputs || !outputs) {
        fprintf(stderr, "Allocation failed for input/output metadata\n");
        cleanup(network, input_buffers, n_inputs, inputs, outputs);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (size_t i = 0; i < n_inputs; i++) {
        size_t input_size = network_get_tensor_size(network, i, TENSOR_TYPE_INPUT);
        uint8_t* input_data = malloc(input_size);
        if (!input_data) {
            fprintf(stderr, "Failed to allocate input buffer\n");
            cleanup(network, input_buffers, i, inputs, outputs);
            return 1;
        }
        fill_input_uint8(input_data, (int) input_size / sizeof(uint8_t), input_mode);
        input_buffers[i] = input_data;
        inputs[i].type = INPUT_DTYPE_UINT8;
        inputs[i].data = input_data;
        inputs[i].size = input_size;
        inputs[i].index = i;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    infer_times[1] = end.tv_nsec - start.tv_nsec;

    for (size_t i = 0; i < repeat; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (!network_predict(network, inputs, n_inputs, outputs, n_outputs)) {
            fprintf(stderr, "Inference failed\n");
            cleanup(network, input_buffers, n_inputs, inputs, outputs);
            return 1;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        double infer_time = end.tv_nsec - start.tv_nsec;
        printf("Predict #%lu: %.2lf\n", i, infer_time / 1e6);
        infer_times[2] += infer_time;
    }

    if (dump_out) {
        for (size_t i = 0; i < n_outputs; i++) {
            char filename[32];
            if (snprintf(filename, sizeof(filename), "output_float_%zu.dat", i) < 0) {
                fprintf(stderr, "Error formatting output filename for output %lu\n", i);
                continue;
            }
            printf("\nWriting denormalized output %zu to file: %s\n", i, filename);
            if (!binary_file_write(filename, outputs[i].data_float, network_get_tensor_item_count(network, 0, TENSOR_TYPE_OUTPUT) * sizeof(float))) {
                fprintf(stderr, "Failed to write output %lu to file: %s\n", i, filename);
                continue;
            }
        }
    }
    if (dump_raw) {
        for (size_t i = 0; i < n_outputs; i++) {
            char filename[32];
            if (snprintf(filename, sizeof(filename), "output_raw_%zu.dat", i) < 0) {
                fprintf(stderr, "Error formatting raw output filename for output %lu\n", i);
                continue;
            }
            printf("\nWriting raw output %zu to file: %s\n", i, filename);
            if (!binary_file_write(filename, outputs[i].data, outputs[i].size)) {
                fprintf(stderr, "Failed to write output %lu to file: %s\n", i, filename);
                continue;
            }
        }
    }

    printf("\nInference times (ms): load: %.2lf, init: %.2lf, predict: %.2lf\n",
           infer_times[0] / 1e6, infer_times[1] / 1e6, infer_times[2] / repeat / 1e6);

    cleanup(network, input_buffers, n_inputs, inputs, outputs);

    return 0;
}

#define _POSIX_C_SOURCE 199309L

#include "synap_cnetwork.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>


/// generate dummy input (zero, constant, or random)
void fill_input_uint8(uint8_t* buffer, size_t size, const char* mode) {
    if (strcmp(mode, "zero") == 0) {
        memset(buffer, 0, size);
    } else if (strncmp(mode, "value:", 6) == 0) {
        int val = atoi(mode + 6);
        for (size_t i = 0; i < size; i++) {
            buffer[i] = (uint8_t)val;
        }
    } else if (strcmp(mode, "random") == 0) {
        for (size_t i = 0; i < size; i++) {
            buffer[i] = rand() % 256;
        }
    } else {
        fprintf(stderr, "Unknown input mode: %s\n", mode);
        exit(1);
    }
}

void cleanup_buffers(uint8_t* buffers[], int count) {
    for (int i = 0; i < count; i++) {
        if (buffers[i]) {
            free(buffers[i]);
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: %s <model_file: .synap> <input_mode: zero | random | value:n>\n", argv[0]);
        return 1;
    }

    const char* model_path = argv[1];
    const char* input_mode = argv[2];
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
    printf("Loaded model: %s\n", model_path);
    size_t n_inputs = network_get_input_count(network);
    size_t n_outputs = network_get_output_count(network);

    uint8_t** input_buffers = calloc(n_inputs, sizeof(uint8_t*));
    CNetworkInput* inputs = calloc(n_inputs, sizeof(CNetworkInput));
    CNetworkOutput* outputs = calloc(n_outputs, sizeof(CNetworkOutput));
    if (!input_buffers || !inputs || !outputs) {
        fprintf(stderr, "Allocation failed for input/output metadata\n");
        free(input_buffers);
        free(inputs);
        free(outputs);
        network_destroy(network);
        return 1;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < n_inputs; i++) {
        size_t input_size = network_get_input_size(network, i);
        uint8_t* input_data = malloc(input_size);
        if (!input_data) {
            fprintf(stderr, "Failed to allocate input buffer\n");
            cleanup_buffers(input_buffers, i);
            free(input_buffers);
            free(inputs);
            free(outputs);
            network_destroy(network);
            return 1;
        }
        fill_input_uint8(input_data, input_size, input_mode);
        input_buffers[i] = input_data;
        inputs[i].data.u8 = input_data;
        inputs[i].size = input_size;
        inputs[i].index = i;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    infer_times[1] = end.tv_nsec - start.tv_nsec;

    clock_gettime(CLOCK_MONOTONIC, &start);
    if (!network_predict(network, inputs, n_inputs, outputs, n_outputs)) {
        fprintf(stderr, "Inference failed\n");
        cleanup_buffers(input_buffers, n_inputs);
        free(input_buffers);
        free(inputs);
        free(outputs);
        network_destroy(network);
        return 1;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    infer_times[2] = end.tv_nsec - start.tv_nsec;

    printf("Output size: %zu floats\n", outputs[0].size / sizeof(float));
    printf("Inference times (ms): load: %lf, init: %lf, predict: %lf\n",
           infer_times[0] / 1e6, infer_times[1] / 1e6, infer_times[2] / 1e6);

    cleanup_buffers(input_buffers, n_inputs);
    free(input_buffers);
    free(inputs);
    free(outputs);
    network_destroy(network);

    return 0;
}

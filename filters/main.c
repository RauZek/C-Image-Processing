#include <errno.h>
#include <getopt.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void sharpening(uint8_t* input, uint8_t* output, int32_t width, int32_t stride,
int32_t height) {
    float kernel[3][3] = {
        {0, -0.5f, 0},
        {-0.5f, 3, -0.5f},
        {0, -0.5f, 0}
    };

    for (int16_t img_y = 1; img_y < height - 1; img_y++) {
        for (int16_t img_x = 1; img_x < width - 1; img_x++) {
            float sum = 0.0f;

            for (int16_t kernel_y = -1; kernel_y <= 1; kernel_y++) {
                for (int16_t kernel_x = -1; kernel_x <= 1; kernel_x++) {
                    int32_t neighbor_x = img_x + kernel_x;
                    int32_t neighbor_y = img_y + kernel_y;

                    sum += input[(neighbor_y * stride) + neighbor_x] *
                           kernel[kernel_y + 1][kernel_x + 1];
                }
            }

            if (sum < 0) sum = 0;
            else if (sum > 255) sum = 255;

            output[(img_y * stride) + img_x] = (uint8_t)(sum + 0.5f);
        }
    }

    for (int16_t y = 0; y < height; y++) {
        for (int16_t x = 0; x < width; x++) {
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                output[y * stride + x] = input[y * stride + x];
            }
        }
    }
}

void sobel_operator(uint8_t* input, uint8_t* output, int32_t width,
                    int32_t stride, int32_t height) {
    int32_t kernel_x[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}
    };

    int32_t kernel_y[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}
    };

    for (int16_t img_y = 1; img_y < height - 1; img_y++) {
        for (int16_t img_x = 1; img_x < width - 1; img_x++) {
            int32_t sum_x = 0;
            int32_t sum_y = 0;

            for (int16_t kr_y = -1; kr_y <= 1; kr_y++) {
                for (int16_t kr_x = -1; kr_x <= 1; kr_x++) {
                    int32_t neighbor_x = img_x + kr_x;
                    int32_t neighbor_y = img_y + kr_y;

                    uint8_t pixel = input[(neighbor_y * stride) + neighbor_x];

                    sum_x += pixel * kernel_x[kr_y + 1][kr_x + 1];
                    sum_y += pixel * kernel_y[kr_y + 1][kr_x + 1];
                }
            }

            float magnitude = sqrtf((float)(sum_x * sum_x + sum_y * sum_y));
            if (magnitude > 255.0f) magnitude = 255.0f;

            output[(img_y * stride) + img_x] = (uint8_t)(magnitude + 0.5f);
        }
    }

    for (int16_t y = 0; y < height; y++) {
        for (int16_t x = 0; x < width; x++) {
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                output[y * stride + x] = input[y * stride + x];
            }
        }
    }
}

void median(uint8_t* input, uint8_t* output, int32_t width, int32_t stride,
            int32_t height) {
    for (uint16_t img_y = 1; img_y < height - 1; img_y++) {
        for (uint16_t img_x = 1; img_x < width - 1; img_x++) {
            uint8_t window[9];
            int16_t index = 0;

            for (int16_t kernel_y = -1; kernel_y <= 1; kernel_y++) {
                for (int16_t kernel_x = -1; kernel_x <= 1; kernel_x++) {
                    int32_t neighbor_x = img_x + kernel_x;
                    int32_t neighbor_y = img_y + kernel_y;
                    window[index++] = input[(neighbor_y * stride) + neighbor_x];
                }
            }

            for (int16_t i = 0; i < 9; i++) {
                for (int16_t j = i + 1; j < 9; j++) {
                    if (window[i] > window[j]) {
                        int8_t temp = window[i];
                        window[i] = window[j];
                        window[j] = temp;
                    }
                }
            }
            output[(img_y * stride) + img_x] = window[4];
        }
    }

    for (int16_t y = 0; y < height; y++) {
        for (int16_t x = 0; x < width; x++) {
            if (y == 0 || y == height - 1 || x == 0 || x == width - 1) {
                output[y * stride + x] = input[y * stride + x];
            }
        }
    }
}

void processing_image(char* input_file, char* output_file, int32_t width,
                      int32_t stride, int32_t height) {
    size_t buffer_size = stride * height;

    uint8_t* input_buffer = malloc(buffer_size);
    uint8_t* output_buffer = calloc(buffer_size, 1);

    if (!input_buffer || !output_buffer) {
        perror("Memory allocation failed.");

        if (input_buffer) {
            free(input_buffer);
        }
        if (output_buffer) {
            free(output_buffer);
        }
        exit(EXIT_FAILURE);
    }

    FILE* input = fopen(input_file, "rb");
    if (!input) {
        perror("Failed to open the input file.");

        free(input_buffer);
        free(output_buffer);
        exit(EXIT_FAILURE);
    }

    if (fread(input_buffer, 1, buffer_size, input) != buffer_size) {
        perror("Failed to read the input file.");
        fclose(input);

        free(input_buffer);
        free(output_buffer);
        exit(EXIT_FAILURE);
    }
    fclose(input);

    sharpening(input_buffer, output_buffer, width, stride, height);

    sobel_operator(output_buffer, input_buffer, width, stride, height);

    median(input_buffer, output_buffer, width, stride, height);

    FILE* output = fopen(output_file, "wb");
    if (!output) {
        perror("Failed to open the output file.");

        free(input_buffer);
        free(output_buffer);
        exit(EXIT_FAILURE);
    }

    if (fwrite(output_buffer, 1, buffer_size, output) != buffer_size) {
        perror("Failed to write to the output file.");
        fclose(output);

        free(input_buffer);
        free(output_buffer);
        exit(EXIT_FAILURE);
    }
    fclose(output);

    free(input_buffer);
    free(output_buffer);
}

int main(int argc, char** argv) {
    char *input_file = NULL, *output_file = NULL;
    int32_t width = 0, stride = 0, height = 0, opt = 0;

    while ((opt = getopt(argc, argv, "i:o:w:s:h:")) != -1) {
        switch (opt) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 's':
                stride = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case '?':
                printf("Unknown option: %c\n", optopt);
                break;
        }
    }
    processing_image(input_file, output_file, width, stride, height);

    return EXIT_SUCCESS;
}

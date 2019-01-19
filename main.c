#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

#ifdef __APPLE__
#   include <OpenCL/cl.h>
#elif
#   include <CL/cl.h>
#endif

#define SHA256_DIGEST_SIZE 32
#define NSEC_PER_SEC 1000000000

typedef struct {
    cl_uint platform_id;
    cl_uint device_id;
    bool copy_buffer;
} config_t;

config_t config = {
    .platform_id = 0,
    .device_id = 0,
    .copy_buffer = false,
};

size_t benchmark_perfect_trees[] = {
        256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
        65536, 131072, 262144, 524288, 1048576, 2097152, 4194304, 8388608, 16777216, 33554432
};
size_t benchmark_imperfect_trees[] = {
        250, 500, 1000, 2000, 4000, 8000, 16000, 32000,
        64000, 128000, 256000, 512000, 1024000, 2048000, 4096000, 8192000, 16384000, 32768000
};

void print_help();
cl_int print_info();
cl_int print_platform_info(cl_platform_id platform, cl_platform_info info);
cl_int print_device_info(cl_device_id device, cl_device_info info);
cl_int find_device(cl_uint platform_id, cl_uint device_id, cl_device_id*);
cl_int run_custom();
cl_int run_benchmark();
cl_int run_file(const char* path);
void print_hash(const uint8_t*);
void print_hash_reversed(const uint8_t*);
bool compile_kernel(cl_context, cl_device_id, cl_program*);
cl_int compute_tree(cl_context, cl_device_id, cl_program, size_t, uint8_t*, bool*);

int main(int argc, char** argv) {
    if(argc < 2){
        printf("Please specify a command!\n\n");
        print_help();
        return 1;
    }

    char* command = argv[1];
    if(strcmp(command, "info") == 0){
        cl_int err = print_info();
        if(err != CL_SUCCESS){
            printf("Encountered OpenCL error %d\n", err);
            return 1;
        }
        return 0;
    }
    if(strcmp(command, "custom") == 0){
        cl_int err = run_custom();
        if(err != CL_SUCCESS){
            printf("Encountered OpenCL error %d\n", err);
            return 1;
        }
        return 0;
    }
    if(strcmp(command, "benchmark") == 0){
        cl_int err = run_benchmark();
        if(err != CL_SUCCESS){
            printf("Encountered OpenCL error %d\n", err);
            return 1;
        }
        return 0;
    }
    if(strcmp(command, "file") == 0){
        if(argc < 3){
            printf("file command requires argument 'path'\n");
            return 1;
        }

        cl_int err = run_file(argv[2]);
        if(err != CL_SUCCESS){
            printf("Encountered OpenCL error %d\n", err);
            return 1;
        }
        return 0;
    }

    printf("Unknown command\n\n");
    print_help();
    return 1;
}

void print_help(){
    printf("./fastmerkle [command]\n");
    printf("\n");
    printf("Commands:\n");
    printf("  info         Prints OpenCL information\n");
    printf("  benchmark    Run benchmark\n");
    printf("  custom       Enter custom values\n");
    printf("  file [path]  Calculate merkle tree from given file\n");
    printf("               Generate files from blocks using the dump2bin.go code\n");
}

cl_int print_info(){
    cl_int err;

    cl_uint num_platforms;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    if(err != CL_SUCCESS) return err;

    cl_platform_id platforms[num_platforms];
    err = clGetPlatformIDs(num_platforms, platforms, NULL);
    if(err != CL_SUCCESS) return err;

    for(cl_uint i = 0; i < num_platforms; i++){
        cl_platform_id platform = platforms[i];

        printf("Platform #%d\n", i);

        printf("  Name:           ");
        print_platform_info(platform, CL_PLATFORM_NAME);
        printf("\n");

        printf("  Version:        ");
        print_platform_info(platform, CL_PLATFORM_VERSION);
        printf("\n\n");

        cl_uint num_devices;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
        if(err != CL_SUCCESS) return err;

        cl_device_id devices[num_devices];
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
        if(err != CL_SUCCESS) return err;

        for(cl_uint j = 0; j < num_devices; j++){
            cl_device_id device = devices[j];

            printf("  Device #%d\n", i);

            printf("    Name:     ");
            print_device_info(device, CL_DEVICE_NAME);
            printf("\n");

            cl_device_type device_type;
            err = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(device_type), &device_type, NULL);
            if(err != CL_SUCCESS) return err;

            printf("    Type:     ");
            if(device_type == CL_DEVICE_TYPE_CPU){
                printf("CPU");
            } else if(device_type == CL_DEVICE_TYPE_GPU){
                printf("GPU");
            } else {
                printf("Other");
            }
            printf("\n\n");
        }
    }

    return CL_SUCCESS;
}

cl_int print_platform_info(cl_platform_id platform, cl_platform_info info){
    cl_int err;

    size_t info_length;
    err = clGetPlatformInfo(platform, info, 0, NULL, &info_length);
    if(err != CL_SUCCESS) return err;

    char data[info_length];
    err = clGetPlatformInfo(platform, info, info_length, data, NULL);
    if(err != CL_SUCCESS) return err;

    printf("%s", data);

    return CL_SUCCESS;
}

cl_int print_device_info(cl_device_id device, cl_device_info info){
    cl_int err;

    size_t info_length;
    err = clGetDeviceInfo(device, info, 0, NULL, &info_length);
    if(err != CL_SUCCESS) return err;

    char data[info_length];
    err = clGetDeviceInfo(device, info, info_length, data, NULL);
    if(err != CL_SUCCESS) return err;

    printf("%s", data);

    return CL_SUCCESS;
}

cl_int find_device(cl_uint platform_id, cl_uint device_id, cl_device_id* device){
    cl_int err;

    cl_platform_id platforms[platform_id + 1];

    err = clGetPlatformIDs(platform_id + 1, platforms, NULL);
    if(err != CL_SUCCESS){
        printf("Unable to get platform id %d\n", platform_id);
        return err;
    }

    cl_device_id devices[device_id + 1];
    err = clGetDeviceIDs(platforms[platform_id], CL_DEVICE_TYPE_ALL, device_id + 1, devices, NULL);
    if(err != CL_SUCCESS){
        printf("Unable to get device id %d\n", device_id);
        return err;
    }

    *device = devices[device_id];

    return CL_SUCCESS;
}

cl_int run_custom(){
    cl_int err;

    printf("Finding OpenCL device\n");

    cl_device_id device;
    err = find_device(config.platform_id, config.device_id, &device);
    if(err != CL_SUCCESS) return err;

    cl_context ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err != CL_SUCCESS){
        printf("Failed to create OpenCL context\n");
        return err;
    }

    printf("Compiling source code\n");

    cl_program program;
    if(!compile_kernel(ctx, device, &program)){
        printf("Failed to compile kernel\n");
        return CL_BUILD_ERROR;
    }

    while(true){
        size_t leaves;

        printf("Enter number of leaves (0 to exit): ");
        scanf("%zu", &leaves);

        if(leaves == 0) break;

        struct timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);

        uint8_t* data = calloc(leaves, SHA256_DIGEST_SIZE * sizeof(uint8_t));
        bool mutated = false;

        err = compute_tree(ctx, device, program, leaves, data, &mutated);
        if(err != CL_SUCCESS){
            free(data);

            printf("Failed to compute tree\n");
            return err;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        long long msec = ((end.tv_sec - start.tv_sec) * NSEC_PER_SEC + (end.tv_nsec - start.tv_nsec)) / 1000000;

        printf("  --> Tree computed!\n");

        printf("  --> Root hash: ");
        print_hash(data);
        printf("\n");

        printf("  --> Mutated: %s\n", mutated ? "true" : "false");

        printf("  --> Took: %lldms\n", msec);

        free(data);
    }

    return CL_SUCCESS;
}

cl_int run_benchmark(){
    assert(sizeof(benchmark_perfect_trees) == sizeof(benchmark_imperfect_trees));

    cl_int err;

    printf("Finding OpenCL device\n");

    cl_device_id device;
    err = find_device(config.platform_id, config.device_id, &device);
    if(err != CL_SUCCESS) return err;

    cl_context ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err != CL_SUCCESS){
        printf("Failed to create OpenCL context\n");
        return err;
    }

    printf("Compiling source code\n");

    cl_program program;
    if(!compile_kernel(ctx, device, &program)){
        printf("Failed to compile kernel\n");
        return CL_BUILD_ERROR;
    }

    printf("Running on device: ");
    print_device_info(device, CL_DEVICE_NAME);
    printf("\n");
    printf("Running on version: ");
    print_device_info(device, CL_DEVICE_VERSION);
    printf("\n");
    printf("Copy Buffer: %s\n", config.copy_buffer ? "enabled" : "disabled");
    printf("\n");

    for(size_t i = 0; i < 2 * sizeof(benchmark_perfect_trees) / sizeof(benchmark_imperfect_trees[0]); i++){
        size_t leaves;
        if(i % 2 == 0){
            leaves = benchmark_perfect_trees[i / 2];
        } else {
            leaves = benchmark_imperfect_trees[(i - 1) / 2];
        }

        printf("Leaves: %8zu", leaves);

        struct timespec start, end;

        clock_gettime(CLOCK_MONOTONIC, &start);

        uint8_t* data = calloc(leaves, SHA256_DIGEST_SIZE * sizeof(uint8_t));
        bool mutated = false;

        err = compute_tree(ctx, device, program, leaves, data, &mutated);
        if(err != CL_SUCCESS){
            free(data);

            printf("\nFailed to compute tree\n");
            return err;
        }

        clock_gettime(CLOCK_MONOTONIC, &end);

        long long msec = ((end.tv_sec - start.tv_sec) * NSEC_PER_SEC + (end.tv_nsec - start.tv_nsec)) / 1000000;

        printf("  Computed in %lldms\n", msec);
        free(data);
    }

    return CL_SUCCESS;
}

cl_int run_file(const char* path){
    printf("Reading block file\n");

    FILE* fp = fopen(path, "r");

    if(fp == NULL){
        perror("Failed to open file");
        return CL_SUCCESS;
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t) ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if(file_size % SHA256_DIGEST_SIZE != 0){
        printf("Invalid block file, invalid number of bytes");
        return CL_SUCCESS;
    }

    uint8_t *data = calloc(file_size, sizeof(uint8_t));
    fread(data, file_size, 1, fp);
    if(fclose(fp) != 0){
        perror("Failed to close file");
        return CL_SUCCESS;
    }

    cl_int err;

    printf("Finding OpenCL device\n");

    cl_device_id device;
    err = find_device(config.platform_id, config.device_id, &device);
    if(err != CL_SUCCESS) return err;

    cl_context ctx = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if(err != CL_SUCCESS){
        printf("Failed to create OpenCL context\n");
        return err;
    }

    printf("Compiling source code\n");

    cl_program program;
    if(!compile_kernel(ctx, device, &program)){
        printf("Failed to compile kernel\n");
        return CL_BUILD_ERROR;
    }

    size_t leaves = file_size / SHA256_DIGEST_SIZE;

    printf("Starting computation\n");

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    bool mutated = false;

    err = compute_tree(ctx, device, program, leaves, data, &mutated);
    if(err != CL_SUCCESS){
        free(data);

        printf("Failed to compute tree\n");
        return err;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);

    long long msec = ((end.tv_sec - start.tv_sec) * NSEC_PER_SEC + (end.tv_nsec - start.tv_nsec)) / 1000000;

    printf("  --> Tree computed!\n");

    printf("  --> Root hash: ");
    print_hash_reversed(data);
    printf("\n");

    printf("  --> Mutated: %s\n", mutated ? "true" : "false");

    printf("  --> Took: %lldms\n", msec);

    free(data);

    return CL_SUCCESS;
}

void print_hash(const uint8_t* hash){
    for(size_t i = 0; i < SHA256_DIGEST_SIZE; i++){
        printf("%02x", hash[i]);
    }
}

void print_hash_reversed(const uint8_t* hash){
    for(int i = SHA256_DIGEST_SIZE - 1; i >= 0; i--){
        printf("%02x", hash[i]);
    }
}

bool compile_kernel(cl_context ctx, cl_device_id device, cl_program* program){
    FILE* fp = fopen("merkle.cl", "r");

    if(fp == NULL){
        perror("Failed to open merkle.cl");
        return false;
    }

    fseek(fp, 0, SEEK_END);
    size_t file_size = (size_t) ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *source_code = calloc(file_size + 1, sizeof(char));
    fread(source_code, file_size, 1, fp);
    if(fclose(fp) != 0){
        perror("Failed to close file merkle.cl");
        return false;
    }

    cl_int err;

    *program = clCreateProgramWithSource(ctx, 1, (const char**)&source_code, &file_size, &err);
    if(err != CL_SUCCESS){
        printf("Failed to create program with souorce");
        return false;
    }

    err = clBuildProgram(*program, 1, &device, NULL, NULL, NULL);
    if(err != CL_SUCCESS){
        printf("Building OpenCL source failed!");

        size_t log_length;
        err = clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_length);
        if(err != CL_SUCCESS){
            printf(" Obtaining build log length failed!\n");
            return false;
        }

        char log[log_length];
        err = clGetProgramBuildInfo(*program, device, CL_PROGRAM_BUILD_LOG, log_length, log, NULL);
        if(err != CL_SUCCESS){
            printf(" Reading build log failed!\n");
            return false;
        }

        printf("Error while building program:\n%s", log);

        return false;
    };

    return true;
}

cl_int compute_tree(cl_context ctx, cl_device_id device, cl_program program, size_t leaves, uint8_t* data, bool* mutated_ptr){
    cl_int err;

    cl_command_queue queue = clCreateCommandQueue(ctx, device, 0, &err);
    if(err != CL_SUCCESS) return err;

    cl_mem data_buf;
    if(config.copy_buffer){
        data_buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE, SHA256_DIGEST_SIZE * leaves * sizeof(uint8_t), NULL, &err);
        if(err != CL_SUCCESS) return err;

        err = clEnqueueWriteBuffer(queue, data_buf, CL_TRUE, 0, SHA256_DIGEST_SIZE * leaves * sizeof(uint8_t), data, 0, NULL, NULL);
        if(err != CL_SUCCESS) return err;
    } else {
        data_buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, SHA256_DIGEST_SIZE * leaves * sizeof(uint8_t), data, &err);
        if(err != CL_SUCCESS) return err;
    }

    cl_int mutated = 0;
    cl_mem mutated_buf = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(cl_int), &mutated, &err);
    if(err != CL_SUCCESS) return err;

    cl_kernel kernel = clCreateKernel(program, "merkle", &err);
    if(err != CL_SUCCESS) return err;

    err = clSetKernelArg(kernel, 0, sizeof(data_buf), &data_buf);
    if(err != CL_SUCCESS) return err;

    err = clSetKernelArg(kernel, 3, sizeof(mutated_buf), &mutated_buf);
    if(err != CL_SUCCESS) return err;

    for(size_t round = 0; leaves > 1; round++){
        err = clSetKernelArg(kernel, 1, sizeof(leaves), &leaves);
        if(err != CL_SUCCESS) return err;

        err = clSetKernelArg(kernel, 2, sizeof(round), &round);
        if(err != CL_SUCCESS) return err;

        const size_t threads = (leaves % 2 == 0 ? leaves : (leaves + 1)) >> 1;
        err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &threads, NULL, 0, NULL, NULL);
        if(err != CL_SUCCESS) return err;

        if(leaves % 2 != 0) leaves++;
        leaves >>= 1;
    }

    cl_event wait;
    err = clEnqueueBarrierWithWaitList(queue, 0, NULL, &wait);
    if(err != CL_SUCCESS) return err;

    err = clWaitForEvents(1, &wait);
    if(err != CL_SUCCESS) return err;

    if(config.copy_buffer){
        err = clEnqueueReadBuffer(queue, data_buf, CL_TRUE, 0, SHA256_DIGEST_SIZE * sizeof(uint8_t), data, 0, NULL, NULL);
        if(err != CL_SUCCESS) return err;
    }

    err = clFinish(queue);
    if(err != CL_SUCCESS) return err;

    if(mutated_ptr != NULL){
        *mutated_ptr = mutated != 0;
    }

    return CL_SUCCESS;
}

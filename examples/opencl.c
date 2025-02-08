#include <sr3am.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <stdio.h>
#include <string.h>

extern const char _binary_examples_opencl_cl_start[];
extern const char _binary_examples_opencl_cl_end[];

int main() {
  // // setup opencl // //
  cl_int err;
  cl_platform_id   platform; // platform owo
  cl_device_id     device_id;     // compute device id
  cl_context       context;       // compute context
  cl_command_queue commandq;      // compute command queue
  cl_program       program;       // compute program
  cl_kernel        compixel;       // compute kernel

  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != CL_SUCCESS) {printf("No Opencl platform\n");return 1;}
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, NULL);
  if (err != CL_SUCCESS) return -2;
  char name[32];
  clGetDeviceInfo(device_id, CL_DEVICE_NAME, 32, &name, NULL);
  printf("%s\n", name);
  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
  if (err != CL_SUCCESS) return 3;
  commandq = clCreateCommandQueue(context, device_id, 0, &err);
  if (err != CL_SUCCESS) return 4;
  size_t clength = _binary_examples_opencl_cl_end - _binary_examples_opencl_cl_start;
  char *clcode = malloc(clength);
  memcpy(clcode, _binary_examples_opencl_cl_start, clength);
  program = clCreateProgramWithSource(context, 1, (const char **) &clcode, &clength, &err);
  if (err != CL_SUCCESS) return 5;
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t len;
    printf("Compile error\n");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    char buffer[len];
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);

    printf("%s\n", buffer);
    return 6;
  }
  compixel = clCreateKernel(program, "pixel", &err);
  if (err != CL_SUCCESS) return 7;

  samImage win = samWindow("opencl test", 480, 480, 0, 0, 0);
  if (win==NULL) {printf("No window\n");return 0;}

  size_t worksize[2];

  // draw
  struct rgba *img;
  uint32_t width, height;

  cl_mem pixelgpu = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 10000000*4, NULL, &err); // 40 megabytes (slighty more than enough for a 4k image)
  cl_mem widthgpu = clCreateBuffer(context, CL_MEM_READ_ONLY, 4, NULL, &err);
  cl_mem heightgpu = clCreateBuffer(context, CL_MEM_READ_ONLY, 4, NULL, &err);
  err = clSetKernelArg(compixel, 0, sizeof(cl_mem), &pixelgpu);
  err = clSetKernelArg(compixel, 1, sizeof(cl_mem), &widthgpu);
  err = clSetKernelArg(compixel, 2, sizeof(cl_mem), &heightgpu);

  while (!samClosing(win)) {
    img = samPixels(&width, &height, win);

    // setup buffers
    clEnqueueWriteBuffer(commandq, widthgpu, CL_TRUE, 0, 4, &width, 0, NULL, NULL);
    if (err != CL_SUCCESS) return 8;
    clEnqueueWriteBuffer(commandq, heightgpu, CL_TRUE, 0, 4, &height, 0, NULL, NULL);
    if (err != CL_SUCCESS) return 8;
    
    worksize[0] = width;
    worksize[1] = height;
    err = clEnqueueNDRangeKernel(commandq, compixel, 2, NULL, worksize, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqeuendrangekernel %i\n", err);
    err = clEnqueueReadBuffer(commandq, pixelgpu, CL_TRUE, 0, width*height*4, img, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqueuereadbuffer %i\n", err);

    samUpdatePerf(win, 1);
    samWait(win);
  }

  clReleaseMemObject(pixelgpu);
  clReleaseMemObject(widthgpu);
  clReleaseMemObject(heightgpu);

  samClose(win);
}
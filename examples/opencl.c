#include <sr3am.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <stdio.h>
#include <string.h>

const char *src = 
"struct rgba {uchar r,g,b,a;};"
"__kernel void pixel(__global struct rgba *img) {"
"int i = get_global_id(0);"
"img[i].r = 255;"
"}";

int main() {
  samImage win = samWindow("opencl test", 480, 480, 0, 0, 0);

  // // setup opencl // //
  cl_int err;
  cl_platform_id   platform; // platform owo
  cl_device_id     device_id;     // compute device id
  cl_context       context;       // compute context
  cl_command_queue commandq;      // compute command queue
  cl_program       program;       // compute program
  cl_kernel        compixel;       // compute kernel

  err = clGetPlatformIDs(1, &platform, NULL);
  if (err != CL_SUCCESS) return -1;
  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, NULL);
  if (err != CL_SUCCESS) return -2;
  context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err);
  if (err != CL_SUCCESS) return -3;
  commandq = clCreateCommandQueue(context, device_id, 0, &err);
  if (err != CL_SUCCESS) return -4;
  size_t clength = strlen(src);
  program = clCreateProgramWithSource(context, 1, (const char **) &src, &clength, &err);
  if (err != CL_SUCCESS) return -5;
  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    size_t len;
    char buffer[2048];

    printf("Compile error\n");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    printf("%s\n", buffer);
    return -6;
  }
  compixel = clCreateKernel(program, "pixel", &err);
  if (err != CL_SUCCESS) return -7;

  cl_mem pixelgpu = clCreateBuffer(context, CL_MEM_WRITE_ONLY, 480*480*4, NULL, &err);
  if (err) return -8;

  err = clSetKernelArg(compixel, 0, sizeof(cl_mem), &pixelgpu);
  if (err != CL_SUCCESS) return -9;

  size_t worksize = 1;

  // draw
  struct rgba *img;
  uint32_t width, height;

  while (!samClosing(win)) {
    img = samPixels(&width, &height, win);

    worksize = width*height;
    err = clEnqueueNDRangeKernel(commandq, compixel, 1, NULL, &worksize, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqeuendrangekernel\n");
    clFinish(commandq);
    err = clEnqueueReadBuffer(commandq, pixelgpu, CL_TRUE, 0, width*height*4, img, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqueuewritebuffer\n");

    samUpdate(win);
    samWait(win);
  }

  samClose(win);
}
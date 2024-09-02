#include <sr3am.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <stdio.h>
#include <string.h>

const char *src = R"(
struct rgba {uchar r,g,b,a;};

__kernel void pixel(__global struct rgba *img, __global unsigned int *width, __global unsigned int *height) {
  float ix = get_global_id(0);
  float iy = get_global_id(1);
  uint i = iy*(*width) + ix;

  float fov = 90 * (3.14159265/180); // 90 degrees in radians
  float aspect_ratio = (float) *width / (float) *height; // Assuming square pixels

  // Calculate horizontal angle
  float horizontal = atan((2 * (ix + 0.5) / (*width) - 1) * tan(fov / 2) * aspect_ratio);

  // Calculate vertical angle
  float vertical = atan((1 - 2 * (iy + 0.5) / (*height)) * tan(fov / 2));

  vertical -= 0.5;

  // Now you have horizontal and vertical angles, you can use them to calculate ray direction
  float3 ray_direction = {
    cos(vertical) * sin(horizontal),
    sin(vertical),
    cos(vertical) * cos(horizontal)
  };

  float3 cam = {100,100,100}; // xyz camera, z=up
  for (uint dist = 0;dist<255;dist++) {
    cam += ray_direction;
    if (cam.y < (sin(cam.x/5)+sin(cam.z/5))*10) {img[i].r = dist;return;};
  }
};
)";

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
  char name[32];
  clGetDeviceInfo(device_id, CL_DEVICE_NAME, 32, &name, NULL);
  printf("%s\n", name);
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
    printf("Compile error\n");
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    char buffer[len];
    clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);

    printf("%s\n", buffer);
    return -6;
  }
  compixel = clCreateKernel(program, "pixel", &err);
  if (err != CL_SUCCESS) return -7;

  size_t worksize[2];

  // draw
  struct rgba *img;
  uint32_t width, height;

  cl_mem pixelgpu;

  while (!samClosing(win)) {
    img = samPixels(&width, &height, win);

    // create buffer
    cl_mem pixelgpu = clCreateBuffer(context, CL_MEM_WRITE_ONLY, width*height*4, NULL, &err);
    if (err != CL_SUCCESS) return -8;
    err = clSetKernelArg(compixel, 0, sizeof(cl_mem), &pixelgpu);
    if (err != CL_SUCCESS) return -9;
    cl_mem widthgpu = clCreateBuffer(context, CL_MEM_READ_ONLY, 4, NULL, &err);
    clEnqueueWriteBuffer(commandq, widthgpu, CL_TRUE, 0, 4, &width, 0, NULL, NULL);
    if (err != CL_SUCCESS) return -8;
    err = clSetKernelArg(compixel, 1, sizeof(cl_mem), &widthgpu);
    if (err != CL_SUCCESS) return -9;
    cl_mem heightgpu = clCreateBuffer(context, CL_MEM_READ_ONLY, 4, NULL, &err);
    clEnqueueWriteBuffer(commandq, heightgpu, CL_TRUE, 0, 4, &height, 0, NULL, NULL);
    if (err != CL_SUCCESS) return -8;
    err = clSetKernelArg(compixel, 2, sizeof(cl_mem), &heightgpu);
    if (err != CL_SUCCESS) return -9;
    
    worksize[0] = width;
    worksize[1] = height;
    err = clEnqueueNDRangeKernel(commandq, compixel, 2, NULL, worksize, NULL, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqeuendrangekernel %i\n", err);
    err = clEnqueueReadBuffer(commandq, pixelgpu, CL_TRUE, 0, width*height*4, img, 0, NULL, NULL);
    if (err != CL_SUCCESS) printf("enqueuereadbuffer %i\n", err);

    clReleaseMemObject(pixelgpu);
    clReleaseMemObject(widthgpu);
    clReleaseMemObject(heightgpu);

    samUpdate(win);
    samWait(win);
  }

  samClose(win);
}
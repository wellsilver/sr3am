#include <sr3am.h>
#define CL_TARGET_OPENCL_VERSION 120
#include <CL/cl.h>

#include <stdio.h>
#include <string.h>

const char *src = R"(
struct rgba {uchar r,g,b,a;};

static int SEED = 0;

static int hash[] = {208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
                     185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
                     9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
                     70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
                     203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
                     164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
                     228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
                     232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
                     193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
                     101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
                     135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
                     114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219};

int noise2(int x, int y) {
  int tmp = hash[(y + SEED) % 256];
  return hash[(tmp + x) % 256];
}

float lin_inter(float x, float y, float s) {
  return x + s * (y-x);
}

float smooth_inter(float x, float y, float s) {
  return lin_inter(x, y, s * s * (3-2*s));
}

float noise2d(float x, float y) {
  int x_int = x;
  int y_int = y;
  float x_frac = x - x_int;
  float y_frac = y - y_int;
  int s = noise2(x_int, y_int);
  int t = noise2(x_int+1, y_int);
  int u = noise2(x_int, y_int+1);
  int v = noise2(x_int+1, y_int+1);
  float low = smooth_inter(s, t, x_frac);
  float high = smooth_inter(u, v, x_frac);
  return smooth_inter(low, high, y_frac);
}

float perlin2d(float x, float y, float freq, int depth) {
  float xa = x*freq;
  float ya = y*freq;
  float amp = 1.0;
  float fin = 0;
  float div = 0.0;

  int i;
  for(i=0; i<depth; i++)
  {
      div += 256 * amp;
      fin += noise2d(xa, ya) * amp;
      amp /= 2;
      xa *= 2;
      ya *= 2;
  }

  return fin/div;
}

__kernel void pixel(__global struct rgba *img, __global unsigned int *width, __global unsigned int *height) {
  float fov = 90 * (3.14159265/180); // 90 degrees in radians
  float aspect_ratio = (float) *width / (float) *height; // Assuming square pixels
  float ix = get_global_id(0);
  float iy = get_global_id(1);
  unsigned int i = iy*(*width) + ix;

  // Calculate horizontal angle
  float horizontal = atan((2 * (ix + 0.5) / (*width) - 1) * tan(fov / 2) * aspect_ratio);
  
  // Calculate vertical angle
  float vertical = atan((1 - 2 * (iy + 0.5) / (*height)) * tan(fov / 2));
  vertical -= 0.5; // pointing down
  if (vertical > 0) return;

  float ray_direction[3] = {
    cos(vertical) * sin(horizontal),
    sin(vertical),
    cos(vertical) * cos(horizontal)
  };
  float rd = 8; // distance

  float cam[3] = {100,500,100}; // xyz camera, z=up
  for (unsigned int dist = 0;dist<255*rd;dist++) {
    cam[0] += ray_direction[0];
    cam[1] += ray_direction[1];
    cam[2] += ray_direction[2];
    //if (cam[1] < (sin(cam[0]/25)+cos(cam[2]/25))*30) {img[i].r = dist/rd;break;};
    //if (cam[1] < (sin(cam[0]/25)+cos(cam[2]/25))*30) {img[i].r = cam[1]+90;break;}
    if (cam[1] < noise2d(cam[0]/100,cam[2]/100)) {img[i].r = cam[1];break;}
  }
};


)";

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

  samImage win = samWindow("opencl test", 480, 480, 0, 0, 0);
  if (win==NULL) {printf("No window\n");return 0;}

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
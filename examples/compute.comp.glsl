#version 460

layout(std430, binding = 0) buffer OutputImage {
  uint data[];
};

layout(push_constant) uniform PushData {
  uint x;
  uint y;
  uint w;
  uint h;
} pc;


void main() {
  uint coord = gl_GlobalInvocationID.x + (gl_GlobalInvocationID.y*pc.w);

  data[coord] = 255;
}
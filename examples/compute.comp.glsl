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
#define imagecoord gl_GlobalInvocationID.x
  uvec2 coord = uvec2(gl_GlobalInvocationID.x % pc.w, gl_GlobalInvocationID.x / pc.w);

  uvec3 color = uvec3(0,0,0);

  data[imagecoord] = color.r | (color.g << 8) | (color.b << 16);
}
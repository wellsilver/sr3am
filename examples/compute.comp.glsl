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

layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

void main() {
  uvec2 coord = uvec2(gl_GlobalInvocationID.x % pc.w, gl_GlobalInvocationID.x / pc.w);
  coord.x += pc.x;

  vec3 color = vec3(float(coord.x) / float(pc.w)*255, 0, 0);

  data[gl_GlobalInvocationID.x] = uint(color.r) | (uint(color.g) << 8) | (uint(color.b) << 16);
}
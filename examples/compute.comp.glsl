#version 460

layout(std430, binding = 0) buffer OutputImage {
  uint data[];
};

layout(push_constant) uniform PushData {
  uint posx;
} pc;


void main() {
  uvec2 coord = gl_GlobalInvocationID.xy;

  data[coord.x] = 255;
}
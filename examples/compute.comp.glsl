#version 460

layout(set = 0, binding = 0, rgba8) uniform writeonly image2D img;

layout(push_constant) uniform PushData {
  uint posx;
} pc;


void main() {
  uvec2 coord = gl_GlobalInvocationID.xy;

  vec4 color = vec4(float(coord.x % 256) / 255.0,
                    float(coord.y % 256) / 255.0,
                    0,
                    1.0);

  imageStore(img, ivec2(coord), color);
}
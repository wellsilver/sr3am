struct rgba {uchar r,g,b,a;};

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
  float rd = 4; // distance

  float cam[3] = {100,300,100}; // xyz camera, z=up
  for (unsigned int dist = 0;dist<255*rd;dist++) {
    cam[0] += ray_direction[0];
    cam[1] += ray_direction[1];
    cam[2] += ray_direction[2];
    //if (cam[1] < (sin(cam[0]/25)+cos(cam[2]/25))*30) {img[i].r = dist/rd;break;};
    if (cam[1] < (sin(cam[0]/25)+cos(cam[2]/25))*30) {img[i].r = cam[1]+90;break;}
  }
};
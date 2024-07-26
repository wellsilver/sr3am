#ifndef sr3am_types_h
#define sr3am_types_h

struct samImage_str {
  void *fd;
  unsigned int closing;
};

typedef void * samImage;

#endif
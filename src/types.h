#ifndef sr3am_types_h
#define sr3am_types_h

struct samImage_str {
  void *fd;
  char closing;
};

typedef void * samImage;

#endif
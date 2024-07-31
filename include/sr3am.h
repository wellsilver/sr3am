#include <stdint.h>

typedef void * samImage;

// Create a new window with width, height, x, y. If width/height is zero or x/y is less than zero they are replaced with some default value. The returned image can be NULL
extern samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y, uint64_t hints);
// Close a window
extern void samClose(samImage window);

// 0 if window doesnt want to close, 1 if window wants to close. while (!samClosing(window)) {}
extern int samClosing(samImage window);

// Wait for nothing, polls events
extern void samWait(samImage window);
// Wait for something to happen
extern void samWaitUser(samImage window);

// Get a writable pointer the pixels to a image, in rgb form. width and height can be NULL
extern void *samPixels(uint32_t *width, uint32_t *height, samImage image);
// New frame. This might resize a image's pixel buffer
extern void samUpdate(samImage image);

// Free's any resource individually
extern void samFree(void *any);
// Free's all resources at once
extern void samCleanup();
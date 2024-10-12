#include <stdint.h>

typedef void * samImage;

// Create a new window with width, height, x, y. If width/height is zero or x/y is less than zero they are replaced with some default value. The returned image can be NULL
extern samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y, uint64_t hints);
// Close a window
extern void samClose(samImage window);

// 0 if window doesnt want to close, 1 if window wants to close. while (!samClosing(window)) {}
extern int samClosing(samImage window);

// Mouse position relative to window
extern void samMouse(samImage window, uint32_t *mouseX, uint32_t *mouseY);

// Wait for nothing, polls events
extern void samWait(samImage window);
// Wait for something to happen
extern void samWaitUser(samImage window);

// Get a writable pointer of the pixels to a image, in traditional rgba with uint8's form. width and height can be NULL
extern void *samPixels(uint32_t *width, uint32_t *height, samImage image);
// New frame. This might invalidate the previous buffer from samPixels()
extern void samUpdate(samImage image);
// New frame. Exact same as samUpdate but returns the ammonut of time it took to generate this frame (in microseconds). If no previous frame it measures since window creation
extern uint64_t samUpdatePerf(samImage image);

// Free's any resource individually
extern void samFree(void *any);
// Free's all resources at once
extern void samCleanup();
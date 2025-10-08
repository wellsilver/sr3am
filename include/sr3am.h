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

// Get the last key pressed (negative if key is released, positive if key is pressed)
extern int32_t samKey(samImage window);

// Wait for nothing, polls events
extern void samWait(samImage window);
// Wait for something to happen
extern void samWaitUser(samImage window);

// Get a writable pointer of the pixels to a image, in traditional rgba with uint8's form. width and height can be NULL
extern void *samPixels(uint32_t *width, uint32_t *height, samImage image);
// New frame. This might invalidate the previous buffer from samPixels()
extern void samUpdate(samImage image);
// New frame. Exact same as samUpdate but returns the ammonut of time it took to generate this frame (in microseconds). If no previous frame it measures since window creation
extern uint64_t samUpdatePerf(samImage image, unsigned int showperf);

// Free's any resource individually
extern void samFree(void *any);
// Free's all resources at once
extern void samCleanup();

enum samKeys {
  sam_null = 0,
  sam_0 = '0',
  sam_1 = '1',
  sam_2 = '2',
  sam_3 = '3',
  sam_4 = '4',
  sam_5 = '5',
  sam_6 = '6',
  sam_7 = '7',
  sam_8 = '8',
  sam_9 = '9',
	sam_a = 'a',
	sam_b = 'b',
	sam_c = 'c',
	sam_d = 'd',
	sam_e = 'e',
	sam_f = 'f',
	sam_g = 'g',
	sam_h = 'h',
	sam_i = 'i',
	sam_j = 'j',
	sam_k = 'k',
	sam_l = 'l',
	sam_m = 'm',
	sam_n = 'n',
	sam_o = 'o',
	sam_p = 'p',
	sam_q = 'q',
	sam_r = 'r',
	sam_s = 's',
	sam_t = 't',
	sam_u = 'u',
	sam_v = 'v',
	sam_w = 'w',
	sam_x = 'x',
	sam_y = 'y',
	sam_z = 'z',
  // TODO: MORE KEYS
};
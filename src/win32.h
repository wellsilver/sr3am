#include <stdint.h>
#include <stdio.h>
#include <windows.h>

unsigned int windows_len = 0;
struct samImage_str **windows = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  switch (uMsg) {
    case WM_DESTROY:
      return 0;
    case WM_PAINT:
      return 0;
    case WM_SIZE:
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
};

samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y) {
  const char CLASS_NAME[]  = "Window Class - SR3AM";
  
  WNDCLASS wc = {};
  wc.lpfnWndProc   = WindowProc;
  wc.hInstance     = GetModuleHandle(NULL);
  wc.lpszClassName = CLASS_NAME;
  
  RegisterClass(&wc);

  // convert our uint8_t char to wchar
  /*
  WCHAR convertedname[strlen(name)];
  for (int loop=0;loop<strlen(name);loop++) convertedname[loop] = name[loop];
  */
  // for some reason it wont let me use the unicode versions

  if (x < 0 || y < 0) {x = CW_USEDEFAULT;y = CW_USEDEFAULT;}
  if (width == 0 || height == 0) {width = 480;height = 480;}

  HWND hwnd = CreateWindowExA(
    0,                              // Optional window styles.
    CLASS_NAME,                     // Window class
    name,    // Window text
    WS_OVERLAPPEDWINDOW,            // Window style

    // Size and position
    x, y, width, height,

    NULL,       // Parent window    
    NULL,       // Menu
    GetModuleHandle(NULL),  // Instance handle
    NULL        // Additional application data
  );
  if (hwnd==NULL) return NULL;

  ShowWindow(hwnd, SW_SHOWNORMAL);

  struct samImage_str *ret = malloc(sizeof(struct samImage_str));
  ret->fd = hwnd;

  // replace the list of windows with a new list of windows that has our window
  unsigned int newwindow_len;
  for (int loop=0;loop<windows_len;loop++)
    if (windows[loop]->fd != NULL) 
      newwindow_len++;
  struct samImage_str **newwindow = malloc(sizeof(samImage)*newwindow_len);
  int distinarg = 0;
  for (int loop=0;loop<windows_len;loop++) {
    if (windows[loop]->fd != NULL) {
      newwindow[distinarg] = windows[loop];
      distinarg++;
    }
  }
  newwindow[distinarg] = ret;
  free(windows);
  windows_len = newwindow_len;
  windows = newwindow;

  return ret;
}

void samClose(samImage window) {
  CloseWindow(((struct samImage_str *) window)->fd);
  free(window);
}

unsigned int samClosing(samImage window) {
  return ((struct samImage_str *) window)->closing;
}
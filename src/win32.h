#include <stdint.h>
#include <stdio.h>
#include <windows.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  struct samImage_str *window = GetPropA(hwnd, "SR3AM");
  switch (uMsg) {
    case WM_DESTROY:
      window->closing = 1;
      return 0;
    case WM_PAINT:
      return 0;
    case WM_SIZE:
      return 0;
  }
  return DefWindowProc(hwnd, uMsg, wParam, lParam);
};

samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y, uint64_t hints) {
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

  SetPropA(hwnd, "SR3AM", ret);

  return ret;
}

void samClose(struct samImage_str *window) {
  CloseWindow(window->fd);
  free(window);
}

unsigned int samClosing(struct samImage_str *window) {
  return window->closing;
}
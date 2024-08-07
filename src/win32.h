#include <stdint.h>
#include <stdio.h>
#include <windows.h>

struct samImage_str {
  void *fd;
  unsigned int closing;

  uint32_t width;
  uint32_t height;
  void *pixels;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  struct samImage_str *window = GetPropA(hwnd, "SR3AM");
  switch (uMsg) {
    case WM_QUIT:
      window->closing = 1;
      return 0;
    case WM_DESTROY:
      window->closing = 1;
      return 0;
    case WM_PAINT:
      return 0;
    case WM_SIZE:
      UINT width = LOWORD(lParam);
      UINT height = HIWORD(lParam);
      window->width = width;
      window->height = height;
      window->pixels = realloc(window->pixels, (width*height)*4);      
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

  struct samImage_str *ret = malloc(sizeof(struct samImage_str));
  ret->fd = hwnd;
  ret->pixels = malloc(1);
  ret->closing = 0;

  SetPropA(hwnd, "SR3AM", ret);

  ShowWindow(hwnd, SW_SHOWNORMAL);

  return ret;
}

void samClose(struct samImage_str *window) {
  CloseWindow(window->fd);
  DestroyWindow(window->fd);
  free(window->pixels);
  free(window);
}

unsigned int samClosing(struct samImage_str *window) {
  return window->closing;
}

void samWait(struct samImage_str *window) {
  MSG msg;
  while (PeekMessageA(&msg, window->fd, 0, 0, PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

void samWaitUser(struct samImage_str *window) {
  MSG msg;
  while (PeekMessageA(&msg, window->fd, 0, 0, PM_REMOVE)) {
    if (msg.message == WM_PAINT) return; // I think wm_paint was meant for this
    TranslateMessage(&msg);
    DispatchMessageA(&msg);
  }
}

void *samPixels(uint32_t *width, uint32_t *height, struct samImage_str *window) {
  if (width != NULL) width = &window->width;
  if (height!= NULL) height= &window->height;
  return window->pixels;
}

void samUpdate(struct samImage_str *window) {
  WINBOOL err;
  LPPAINTSTRUCT lppaint;
  HDC hdc = BeginPaint(window->fd, lppaint);
  HBITMAP bmp = CreateBitmap(window->width, window->height, 1, 24, window->pixels); // apparently this is slow, but its the easiest solution for now
  HDC bithdc = CreateCompatibleDC(hdc);
  SelectObject(bithdc, bmp);
  BitBlt(hdc, 0, 0, window->width,window->height, bithdc, 0, 0, SRCCOPY);

  DeleteObject(bmp);
  DeleteDC(bithdc);
  EndPaint(window->fd, lppaint);
}
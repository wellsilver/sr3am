#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>

struct samImage_str {
  Display *dis;
  int screen;
  Window win;
  GC gc;

  Atom wm_protocols;
  Atom wm_deletewin;

  Pixmap bitmap;
  void *bitmapdata;

  int closing;
};

samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y, uint64_t hints) {
  unsigned long white,black;
  struct samImage_str *ret = malloc(sizeof(struct samImage_str));

  if (x < 0 || y < 0) {x = 0;y = 0;}
  if (width == 0 || height == 0) {width = 480;height = 480;}
  int borderwidth=5;

	ret->dis = XOpenDisplay((char *)0);
  ret->screen = DefaultScreen(ret->dis);
  black=BlackPixel(ret->dis, ret->screen),	/* get color black */
	white=WhitePixel(ret->dis, ret->screen);  /* get color white */
  ret->win=XCreateSimpleWindow(ret->dis,DefaultRootWindow(ret->dis),x,y,	
  width, height, borderwidth, white, black);
  XSetStandardProperties(ret->dis, ret->win, name, "", None, NULL, 0, NULL);
  ret->gc=XCreateGC(ret->dis, ret->win, 0,0);
  XSetBackground(ret->dis,ret->gc,white);
	XSetForeground(ret->dis,ret->gc,black);

	XClearWindow(ret->dis, ret->win);
	XMapRaised(ret->dis, ret->win);

  // setup with a window manager
  //ret->wm_protocols = XInternAtom(ret->dis, "WM_PROTOCOLS", False);
  ret->wm_deletewin = XInternAtom(ret->dis, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(ret->dis, ret->win, &ret->wm_deletewin, 1);

  ret->closing = 0;

  return ret;
}

void samClose(struct samImage_str *window) {
  XFreeGC(window->dis, window->gc);
	XDestroyWindow(window->dis,window->win);
	XCloseDisplay(window->dis);	
  free(window);
}

void samWait(struct samImage_str *window) {
  XEvent event;
  while (XPending(window->dis) > 0) {
    XNextEvent(window->dis, &event);
    if (event.type == ClientMessage && event.xclient.data.l[0] == window->wm_deletewin)
      window->closing = True;
  }
}

void samWaitUser(struct samImage_str *window) {
  XEvent event;
  while (1) {
    XNextEvent(window->dis, &event);
    if (event.type == ClientMessage && event.xclient.data.l[0] == window->wm_deletewin)
      window->closing = True;
  }
}

int samClosing(struct samImage_str *window) {
  return window->closing;
}
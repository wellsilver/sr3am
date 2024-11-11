#ifndef x11_h
#define x11_h

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
  Visual *vis;

  Atom wm_protocols;
  Atom wm_deletewin;

  Pixmap bitmap;
  unsigned char *bitmapdata;
  uint32_t width,nwidth;   // the new width
  uint32_t height,nheight; // the new height

  struct timeval lastf;

  int outofdate;

  char keys[8]; // max of 8 keys being pressed, most keyboards are less than 4 so.
  uint32_t mousex;
  uint32_t mousey;

  int closing;
};

samImage samWindow(char *name, uint32_t width, uint32_t height, int32_t x, int32_t y, uint64_t hints) {
  unsigned long white,black;
  struct samImage_str *ret = malloc(sizeof(struct samImage_str));

  if (x < 0 || y < 0) {x = 0;y = 0;}
  if (width == 0 || height == 0) {width = 480;height = 480;}
  int borderwidth=5;

	ret->dis = XOpenDisplay((char *)0);
  if (ret->dis == 0) return NULL;
  ret->screen = DefaultScreen(ret->dis);
  black=BlackPixel(ret->dis, ret->screen),	/* get color black */
	white=WhitePixel(ret->dis, ret->screen);  /* get color white */
  ret->vis = DefaultVisual(ret->dis, 0);
  XSetWindowAttributes wat = { 0 }; // Window ATribbutes, lmao
  wat.colormap = XCreateColormap(ret->dis, DefaultRootWindow(ret->dis), ret->vis, AllocNone);
  wat.event_mask = StructureNotifyMask | KeyPressMask | KeyReleaseMask |
                  PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                  ExposureMask | FocusChangeMask | VisibilityChangeMask |
                  EnterWindowMask | LeaveWindowMask | PropertyChangeMask;
  ret->win=XCreateWindow(ret->dis,DefaultRootWindow(ret->dis),x,y, width, height, 0, DefaultDepth(ret->dis, 0), InputOutput, ret->vis, CWBorderPixel | CWColormap | CWEventMask, &wat);
  XSetStandardProperties(ret->dis, ret->win, name, "", None, NULL, 0, NULL);
  ret->gc=XCreateGC(ret->dis,ret->win,0,0);
  XSetBackground(ret->dis,ret->gc,white);
	XSetForeground(ret->dis,ret->gc,black);

	//XClearWindow(ret->dis, ret->win);
	XMapRaised(ret->dis, ret->win);

  // setup with a window manager
  //ret->wm_protocols = XInternAtom(ret->dis, "WM_PROTOCOLS", False);
  ret->wm_deletewin = XInternAtom(ret->dis, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(ret->dis, ret->win, &ret->wm_deletewin, 1);

  ret->bitmapdata = malloc(width*height*4);
  ret->width = width;
  ret->height = height;

  ret->closing = 0;

  ret->outofdate = 0;

  for (int loop=0;loop<8;loop++) ret->keys[loop] = 0;
  ret->mousex = 0;
  ret->mousey = 0;

  gettimeofday(&ret->lastf, NULL);

  return ret;
}

void samClose(struct samImage_str *window) {
  XFreeGC(window->dis, window->gc);
	XDestroyWindow(window->dis,window->win);
	XCloseDisplay(window->dis);	
  free(window);
}

void proccessevent(struct samImage_str *window, XEvent event) {
  XNextEvent(window->dis, &event);
  if (event.type == ClientMessage && event.xclient.data.l[0] == window->wm_deletewin)
    window->closing = True;
  if (event.type == ConfigureNotify) {
    window->outofdate = 1;
    window->nwidth = event.xconfigure.width;
    window->nheight= event.xconfigure.height;
  }
  if (event.type == MotionNotify) {
    window->mousex = event.xmotion.x;
    window->mousey = event.xmotion.y;
  }
  if (event.type == KeyPress) {
    
  }
}

void samWait(struct samImage_str *window) {
  XEvent event;
  while (XPending(window->dis) > 0) proccessevent(window, event);
}

void samWaitUser(struct samImage_str *window) {
  XEvent event;
  while (1) proccessevent(window, event);
}

void samMouse(struct samImage_str *window, uint32_t *mx, uint32_t *my) {
  if (mx != NULL) *mx = window->mousex;
  if (my != NULL) *my = window->mousey;
}

void *samPixels(uint32_t *width, uint32_t *height, struct samImage_str *image) {
  if (width != NULL) *width = image->width;
  if (height!= NULL) *height= image->height;
  return image->bitmapdata;
}

struct rgba {unsigned char r,g,b,a;};

void samUpdate(struct samImage_str *window) {
  struct rgba *pixels = (struct rgba *) window->bitmapdata;
  for (int loop=0;loop<window->width*window->height;loop++) {
    unsigned char r = pixels[loop].r;
    unsigned char b = pixels[loop].b;
    pixels[loop].b = r;
    pixels[loop].r = b;
  }
  XImage *ximage = XCreateImage(window->dis, window->vis, DefaultDepth(window->dis,window->screen), ZPixmap, 0, (char *) pixels, window->width, window->height, 32, 0);
  XPutImage(window->dis, window->win, window->gc, ximage, 0, 0, 0, 0, window->width, window->height);
  memset(pixels, 0, window->width*window->height*4);

  if (window->outofdate) {
    // will haved to create a new image
    free(window->bitmapdata);
    window->width = window->nwidth;
    window->height = window->nheight;
    window->bitmapdata = malloc(window->width*window->height*4);
    window->outofdate = 0;
  }
  XFlush(window->dis);
}

uint64_t samUpdatePerf(struct samImage_str *window) {
  struct rgba *pixels = (struct rgba *) window->bitmapdata;
  for (int loop=0;loop<window->width*window->height;loop++) {
    unsigned char r = pixels[loop].r;
    unsigned char b = pixels[loop].b;
    pixels[loop].b = r;
    pixels[loop].r = b;
  }
  XImage *ximage = XCreateImage(window->dis, window->vis, DefaultDepth(window->dis,window->screen), ZPixmap, 0, (char *) pixels, window->width, window->height, 32, 0);
  XPutImage(window->dis, window->win, window->gc, ximage, 0, 0, 0, 0, window->width, window->height);
  memset(pixels, 0, window->width*window->height*4);

  // get time
  struct timeval t1;
  gettimeofday(&t1, NULL);
  uint64_t timetaken = ((t1.tv_sec - window->lastf.tv_sec) * 1000000) + (t1.tv_usec - window->lastf.tv_usec); // ((current seconds - previous seconds) -> usec) + (current usec - last usec)
  window->lastf = t1;

  if (window->outofdate) {
    // will haved to create a new image
    free(window->bitmapdata);
    window->width = window->nwidth;
    window->height = window->nheight;
    window->bitmapdata = malloc(window->width*window->height*4);
    window->outofdate = 0;
  }
  XFlush(window->dis);
  // return the ammount of time inbetween last frame and the end of this one in usec's
  return timetaken;
}

/*
void samUpdate(struct samImage_str *window) {
  struct rgba *pixels = (struct rgba *) window->bitmapdata;
  struct rgba *newpixels = malloc(window->width*window->height*4);
  for (int loop=0;loop<window->width*window->height;loop++) {
    newpixels[loop].b = pixels[loop].r;
    newpixels[loop].r = pixels[loop].b;
  }
  XImage *ximage = XCreateImage(window->dis, window->vis, DefaultDepth(window->dis,window->screen), ZPixmap, 0, (char *) newpixels, window->width, window->height, 32, 0);
  XPutImage(window->dis, window->win, window->gc, ximage, 0, 0, 0, 0, window->width, window->height);
  free(newpixels);
}*/

int samClosing(struct samImage_str *window) {
  return window->closing;
}

#endif
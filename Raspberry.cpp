#include "Window.h"

// Raspberry Pi OpenGL window backend
//

#include <fcntl.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>

#include "EGL/egl.h"
#include "EGL/eglext.h"
#include "GLES2/gl2.h"  // OpenGL!
#include "bcm_host.h"
#include "revision.h"

#include "bcm_host.h"  // bcm_host_init / bcm_host_deinit

struct _Window {
  EGLContext context;
  EGLDisplay display;
  EGLSurface surface;
  unsigned width, height;

  void init() {
    bcm_host_init();

    // XXX free? shutdown?
    memset(&context, 0, sizeof(context));
    memset(&display, 0, sizeof(display));
    memset(&surface, 0, sizeof(surface));
    width = height = 0;

    int32_t success = 0;
    EGLBoolean result;
    EGLint num_config;

    static EGL_DISPMANX_WINDOW_T nativewindow;

    DISPMANX_ELEMENT_HANDLE_T dispman_element;
    DISPMANX_DISPLAY_HANDLE_T dispman_display;
    DISPMANX_UPDATE_HANDLE_T dispman_update;
    VC_RECT_T dst_rect;
    VC_RECT_T src_rect;

    static const EGLint attribute_list[] = {EGL_RED_SIZE,     8,
                                            EGL_GREEN_SIZE,   8,
                                            EGL_BLUE_SIZE,    8,
                                            EGL_ALPHA_SIZE,   8,
                                            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
                                            EGL_NONE};

    EGLConfig config;

    // get an EGL display connection
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    assert(display != EGL_NO_DISPLAY);
    check_gl_error();

    // initialize the EGL display connection
    result = eglInitialize(display, NULL, NULL);
    assert(EGL_FALSE != result);
    check_gl_error();

    // get an appropriate EGL frame buffer configuration
    result = eglChooseConfig(display, attribute_list, &config, 1, &num_config);
    assert(EGL_FALSE != result);
    check_gl_error();

    // get an appropriate EGL frame buffer configuration
    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(EGL_FALSE != result);
    check_gl_error();

    // create an EGL rendering context
    {
      static const EGLint attribute_list[] = {EGL_CONTEXT_CLIENT_VERSION, 2,
                                              EGL_NONE};
      context =
          eglCreateContext(display, config, EGL_NO_CONTEXT, attribute_list);
    }
    assert(context != EGL_NO_CONTEXT);
    check_gl_error();

    // create an EGL window surface
    success = graphics_get_display_size(0 /* LCD */, &width, &height);
    assert(success >= 0);

    dst_rect.x = 0;
    dst_rect.y = 0;
    dst_rect.width = width;
    dst_rect.height = height;

    src_rect.x = 0;
    src_rect.y = 0;
    src_rect.width = width << 16;
    src_rect.height = height << 16;

    dispman_display = vc_dispmanx_display_open(0 /* LCD */);
    dispman_update = vc_dispmanx_update_start(0);

    dispman_element = vc_dispmanx_element_add(
        dispman_update, dispman_display, 0 /*layer*/, &dst_rect, 0 /*src*/,
        &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0 /*clamp*/,
        (DISPMANX_TRANSFORM_T)0 /*transform*/);

    nativewindow.element = dispman_element;
    nativewindow.width = width;
    nativewindow.height = height;
    vc_dispmanx_update_submit_sync(dispman_update);

    check_gl_error();

    surface = eglCreateWindowSurface(display, config, &nativewindow, NULL);
    assert(surface != EGL_NO_SURFACE);
    check_gl_error();

    // connect the context to the surface
    result = eglMakeCurrent(display, surface, surface, context);
    assert(EGL_FALSE != result);
    check_gl_error();
  }

  void step() {
    eglSwapBuffers(display, surface);
    check_gl_error();
  }

  // void make(unsigned x, unsigned y, unsigned width, unsigned height) { }

  void halt() {
    bcm_host_deinit();
  }

  bool done() {
    return false;
  }

  void size(int& w, int& h) {
    w = width;
    h = height;
  }
};

//
//
//
Window::Window() { implementation = new _Window(); }
void Window::init() { implementation->init(); }
void Window::step() { implementation->step(); }
void Window::halt() { implementation->halt(); }
bool Window::done() { return implementation->done(); }
void Window::size(int& w, int& h) { implementation->size(w, h); }

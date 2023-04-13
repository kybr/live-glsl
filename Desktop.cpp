#include "Window.h"

#include <cstdio>
#include <cstdlib>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>


static void error_callback(int error, const char* description) {
  fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action,
                         int mods) {
  // XXX change this to FULLSCREEN
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

struct _Window {
  GLFWwindow* window;

  void init() {
    if (!glfwInit()) {
      exit(1);
    }
    glfwSetErrorCallback(error_callback);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    window = glfwCreateWindow(640, 480, "PIG", NULL, NULL);
    if (!window) {
      // Window or OpenGL context creation failed
    }
    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);

    // (aka 'void (*(const char *))()') to (aka 'void *(*)(const char *)')
    gladLoadGLES2Loader((void* (*)(const char*))glfwGetProcAddress);
    glfwSwapInterval(1);
  }

  void step() {
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  void halt() {
    glfwDestroyWindow(window);
    glfwTerminate();
  }

  void size(int& w, int& h) { glfwGetFramebufferSize(window, &w, &h); }

  bool done() { return glfwWindowShouldClose(window); }
};

//
//
//
void Window::init() { implementation->init(); }
void Window::step() { implementation->step(); }
void Window::halt() { implementation->halt(); }
bool Window::done() { return implementation->done(); }
void Window::size(int& w, int& h) { implementation->size(w, h); }

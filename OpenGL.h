#pragma once

// use this header on the desktop; we want the API to
// remain aligned with the Raspberry Pi OpenGL API. we
// might also refine the glad configuration so that
// glad is all we need. this way, we could have this
// single header for both platforms.
#include <glad/glad.h>

// put OpenGL helpers here...
// TBD: add shader management class

void _check_gl_error(const char *file, int line) {
  GLenum err(glGetError());

  while (err != GL_NO_ERROR) {
    std::string error;

    switch (err) {
      case GL_INVALID_OPERATION:
        error = "INVALID_OPERATION";
        break;
      case GL_INVALID_ENUM:
        error = "INVALID_ENUM";
        break;
      case GL_INVALID_VALUE:
        error = "INVALID_VALUE";
        break;
      case GL_OUT_OF_MEMORY:
        error = "OUT_OF_MEMORY";
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        error = "INVALID_FRAMEBUFFER_OPERATION";
        break;
    }

    std::cerr << "GL_" << error.c_str() << " - " << file << ":" << line
              << std::endl;
    err = glGetError();
  }
}

void _check_gl_error(const char *file, int line);
#define check_gl_error() _check_gl_error(__FILE__, __LINE__)

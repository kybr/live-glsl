#include <lo/lo.h>
#include <lo/lo_cpp.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "Window.h"
#include "OpenGL.h"


int ROW = 0, COLUMN = 0;          // location within the 16 displays
int WIDTH = 1920, HEIGHT = 1080;  // of each screen
int BIAS_X = 0, BIAS_Y = 0;       // offset of each screen in pixels

///////////////////////////////////////////////////////////////////////////////
//// OpenGL ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

struct OpenGLState {
  GLuint program_copy;
  GLuint program_copy_attribute_vertex;
  GLuint program_copy_uniform_texture;

  GLuint verbose;
  GLuint shader_fragment_copy;
  GLuint texture_framebuffer;
  GLuint texture;

  GLuint buffer_vertex_array;

  GLuint program;
  GLuint shader_vertex;
  GLuint shader_fragment;
  GLuint attribute_vertex;
  GLuint uniform_texture;
  GLuint uniform_time;
  GLuint uniform_position;
  GLuint uniform_size;
  GLuint uniform_bias;
};

const GLchar *shader_source_default_vertex = R"(
attribute vec4 vertex;
varying vec2 texture_coordinate;
void main(void) {
  vec4 pos = vertex;
  gl_Position = pos;
  texture_coordinate = vertex.xy*0.5+0.5;
}
)";

// Mandelbrot
const GLchar *shader_source_default_fragment = R"(
uniform sampler2D texture;
uniform float time;  
uniform ivec2 position;  
uniform ivec2 size;       
uniform ivec2 bias;              
varying vec2 texture_coordinate;
void main(void) {
  float cr = (gl_FragCoord.x) * 0.003;
  float ci = (gl_FragCoord.y) * 0.003;
  float ar = cr;
  float ai = ci;
  float tr, ti;
  float col = 0.0;
  float p = 0.0;
  int i = 0;
  for (int i2 = 1; i2 < 16; i2++) {
    tr = ar * ar - ai * ai + cr;
    ti = 2.0 * ar * ai + ci;
    p = tr * tr + ti * ti;
    ar = tr;
    ai = ti;
    if (p > 16.0) {
      i = i2;
      break;
    }
  }
  gl_FragColor = vec4(float(i) * 0.0625, 0, 0, 1);
}
)";

static void showlog(GLint shader) {
  // Prints the compile log for a shader
  char log[1024];
  glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
  printf("%d:shader:\n%s\n", shader, log);
  fflush(stdout);
}

static void showprogramlog(GLint shader) {
  // Prints the information log for a program object
  char log[1024];
  glGetProgramInfoLog(shader, sizeof(log), nullptr, log);
  printf("%d:program:\n%s\n", shader, log);
  fflush(stdout);
}

static void compile_shader(OpenGLState *state) {
  state->shader_vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(state->shader_vertex, 1, &shader_source_default_vertex, 0);
  glCompileShader(state->shader_vertex);
  check_gl_error();

  if (state->verbose) showlog(state->shader_vertex);

  state->shader_fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(state->shader_fragment, 1, &shader_source_default_fragment, 0);
  glCompileShader(state->shader_fragment);
  check_gl_error();

  if (state->verbose) showlog(state->shader_fragment);

  state->program = glCreateProgram();
  glAttachShader(state->program, state->shader_vertex);
  glAttachShader(state->program, state->shader_fragment);
  glLinkProgram(state->program);
  check_gl_error();

  if (state->verbose) showprogramlog(state->program);

  // to save on GPU memory!!! XXX
  {
    GLint result = 0;
    glGetProgramiv(state->program, GL_LINK_STATUS, &result);
    if (result == GL_TRUE) {
      glDetachShader(state->program, state->shader_vertex);
      glDetachShader(state->program, state->shader_fragment);
    }
  }

  // save GPU memory
  // glDeleteShader(state->shader_vertex);
  glDeleteShader(state->shader_fragment);

  state->attribute_vertex = glGetAttribLocation(state->program, "vertex");
  state->uniform_texture = glGetUniformLocation(state->program, "texture");
  state->uniform_time = glGetUniformLocation(state->program, "time");
  state->uniform_position = glGetUniformLocation(state->program, "position");
  state->uniform_size = glGetUniformLocation(state->program, "size");
  state->uniform_bias = glGetUniformLocation(state->program, "bias");
  check_gl_error();
}

void more_setup(OpenGLState *state, int width, int height) {
  glClearColor(0.0, 1.0, 1.0, 1.0);

  glGenBuffers(1, &state->buffer_vertex_array);

  check_gl_error();

  //
  {
    // Prepare a texture image
    glGenTextures(1, &state->texture);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, state->texture);
    check_gl_error();
    // glActiveTexture(0)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_SHORT_5_6_5, 0);
    check_gl_error();
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    check_gl_error();
    // Prepare a framebuffer for rendering
    glGenFramebuffers(1, &state->texture_framebuffer);
    check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, state->texture_framebuffer);
    check_gl_error();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           state->texture, 0);
    check_gl_error();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    check_gl_error();
  }

  // Prepare viewport
  glViewport(0, 0, width, height);
  check_gl_error();

  static const GLfloat vertex_data[] = {-1.0, -1.0, 1.0, 1.0, 1.0, -1.0,
                                        1.0,  1.0,  1.0, 1.0, 1.0, 1.0,
                                        -1.0, 1.0,  1.0, 1.0};

  // Upload vertex data to a buffer
  glBindBuffer(GL_ARRAY_BUFFER, state->buffer_vertex_array);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
               GL_STATIC_DRAW);

  glVertexAttribPointer(state->attribute_vertex, 4, GL_FLOAT, 0, 16, 0);
  glEnableVertexAttribArray(state->attribute_vertex);

  check_gl_error();
}

static void render(OpenGLState *state, double ellapsed,
                   bool render_to_texture) {
  // Now render to the main frame buffer
  if (render_to_texture) {
    glBindFramebuffer(GL_FRAMEBUFFER, state->texture_framebuffer);
  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Clear the background (not really necessary I suppose)
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  check_gl_error();

  glBindBuffer(GL_ARRAY_BUFFER, state->buffer_vertex_array);  //?
  check_gl_error();

  glUseProgram(state->program);
  check_gl_error();

  if (render_to_texture) {
    glBindTexture(GL_TEXTURE_2D, state->texture);
    check_gl_error();
  }
  // glUniform2f(state->uniform_scale, 0.003, 0.003);
  // glUniform2f(state->uniform_center, 1920 / 2.0, 1080 / 2.0);
  glUniform1i(state->uniform_texture, 0);
  glUniform1f(state->uniform_time, ellapsed);
  glUniform2i(state->uniform_position, 0, 0);
  glUniform2i(state->uniform_size, 1920, 1080);
  glUniform2i(state->uniform_bias, 0, 0);
  check_gl_error();

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  check_gl_error();

  // glBindBuffer(GL_ARRAY_BUFFER, 0);
  // glUseProgram(0);
  // glFlush();
  // glFinish();
  // check_gl_error();

  if (render_to_texture) {
    // put the texture onto the screen now....
    //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);  // render to the screen
    glUseProgram(state->program_copy);
    check_gl_error();
    glBindTexture(GL_TEXTURE_2D, state->texture);
    check_gl_error();
    glUniform1i(state->program_copy_uniform_texture, 0);
    check_gl_error();
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    check_gl_error();
  }

  glFlush();
  glFinish();
  check_gl_error();
}

void recompile_shader(OpenGLState *state, const char *FRAGMENT) {
  GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, (const GLchar **)&FRAGMENT, 0);
  glCompileShader(fragment);
  check_gl_error();

  if (state->verbose) showlog(fragment);

  GLuint program = glCreateProgram();
  glAttachShader(program, state->shader_vertex);  // vertex already compiled
  glAttachShader(program, fragment);
  glLinkProgram(program);
  check_gl_error();

  if (state->verbose) showprogramlog(program);

  {
    GLint result = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
      glDetachShader(program, state->shader_vertex);
      glDetachShader(program, fragment);
      glDeleteShader(fragment);
      glDeleteProgram(program);
      return;
    }
  }

  // XXX need to automatically figure out these...
  state->attribute_vertex = glGetAttribLocation(state->program, "vertex");
  state->uniform_texture = glGetUniformLocation(state->program, "texture");
  state->uniform_time = glGetUniformLocation(state->program, "time");
  state->uniform_position = glGetUniformLocation(state->program, "position");
  state->uniform_size = glGetUniformLocation(state->program, "size");
  state->uniform_bias = glGetUniformLocation(state->program, "bias");
  check_gl_error();

  glDetachShader(program, state->shader_vertex);
  glDetachShader(program, fragment);
  glDeleteShader(fragment);
  glDeleteProgram(state->program);
  state->program = program;
}

void compile_copy_shader(OpenGLState *state) {
  const GLchar *shader_source_copy_vertex = R"(
attribute vec4 a_Position;
varying highp vec2 v_TexCoordinate;
void main() {
  v_TexCoordinate = a_Position.xy * 0.5 + 0.5;
  // v_TexCoordinate.x = 1.0 - v_TexCoordinate.x;
  gl_Position = a_Position;
}
)";

  GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &shader_source_copy_vertex, 0);
  glCompileShader(vertex);
  check_gl_error();

  if (state->verbose) showlog(state->shader_vertex);

  const GLchar *shader_source_copy_fragment = R"(
uniform sampler2D u_Texture;
varying highp vec2 v_TexCoordinate;
void main() {
  gl_FragColor = texture2D(u_Texture, v_TexCoordinate);
}
)";

  GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &shader_source_copy_fragment, 0);
  glCompileShader(fragment);
  check_gl_error();

  if (state->verbose) showlog(fragment);

  GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  check_gl_error();

  if (state->verbose) showprogramlog(program);

  {
    GLint result = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result != GL_TRUE) {
      glDetachShader(program, vertex);
      glDetachShader(program, fragment);
      glDeleteShader(fragment);
      glDeleteProgram(program);
      return;
    }
  }

  state->program_copy_attribute_vertex =
      glGetAttribLocation(program, "a_Position");
  state->program_copy_uniform_texture =
      glGetUniformLocation(program, "u_Texture");
  check_gl_error();

  glDetachShader(program, vertex);
  glDetachShader(program, fragment);
  glDeleteShader(fragment);
  glDeleteShader(vertex);
  glDeleteProgram(state->program_copy);
  state->program_copy = program;
}

//==============================================================================

struct ClassTimer {
  std::chrono::high_resolution_clock::time_point begin;
  ClassTimer() : begin(std::chrono::high_resolution_clock::now()) {}
  double ellapsed() {
    return std::chrono::duration<double>(
               std::chrono::high_resolution_clock::now() - begin)
        .count();
  }
};

int main(int argc, char *argv[]) {
  lo::ServerThread server("7770");
  ClassTimer class_timer;
  Window window;

  window.init();

  OpenGLState state;
  std::atomic<bool> recompile{false};
  bool render_to_texture = false;

  char FRAGMENT[65536];

  server.add_method("/t", "i", [&](lo_arg **argv, int) {
    render_to_texture = argv[0]->i != 0;
    if (render_to_texture)
      printf("Rendering to texture\n");
    else
      printf("Rendering to directly to screen\n");
    fflush(stdout);
  });

  server.add_method("/s", "ib", [&](lo_arg **argv, int) {
    int version = argv[0]->i;
    int size = argv[1]->blob.size;
    printf("glsl blob %d bytes (version %d)\n", size, version);
    const char *blob = &argv[1]->blob.data;
    memcpy(FRAGMENT, blob, size);
    FRAGMENT[size] = '\0';
    printf("====== fragment ========================\n%s\n", FRAGMENT);
    fflush(stdout);
    recompile = true;
  });

  server.add_method("/bias", "iiii", [&](lo_arg **argv, int) {
    // XXX RC versus XY ?? order inconsistant
    if (argv[0]->i == ROW)
      if (argv[1]->i == COLUMN) {
        BIAS_X = argv[2]->i;
        BIAS_Y = argv[3]->i;
      }
  });

  // server.add_method(nullptr, nullptr, [](lo_arg **argv, int) {
  //   printf("got an OSC message\n");
  //   fflush(stdout);
  // });

  server.start();

  memset(&state, 0, sizeof(OpenGLState));
  state.verbose = 1;

  compile_shader(&state);
  int width, height;
  window.size(width, height);
  more_setup(&state, width, height);
  compile_copy_shader(&state);

  while (!window.done()) {
    window.size(width, height);

    // glViewport(0, 0, width, height);
    // glClearColor(255, 0, 255, 255);
    // glClear(GL_COLOR_BUFFER_BIT);

    if (recompile) {
      recompile = false;
      recompile_shader(&state, FRAGMENT);
      check_gl_error();
    }

    render(&state, class_timer.ellapsed(), render_to_texture);

    window.step();

    check_gl_error();
  }

  window.halt();
  return 0;
}

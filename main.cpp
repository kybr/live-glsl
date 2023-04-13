#include <glad/glad.h>
#include <cmath>
#include "Window.h"

int main(int argc, char* argv[]) {
  Window window;
  window.init();

  while (!window.done()) {
    int width, height;
    window.size(width, height);

    glViewport(0, 0, width, height);
    glClearColor(255, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT);

    window.step();
  }

  window.halt();
  return 0;
}

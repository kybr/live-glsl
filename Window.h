#pragma once

class _Window;
class Window {
  _Window* implementation;

 public:
  Window();
  void init(); // initialize and configure an OpenGL window
  void step(); // swap buffers and any post-frame work
  void halt(); // destroy context and close window
  bool done(); // has the user requested shutdown?
  void size(int& w, int& h); // the size of the window

  // XXX: not necessary, but could add...
  // * init with width and height arguments
  // * resize callback
};

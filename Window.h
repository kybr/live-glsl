#pragma once

class _Window;
class Window {
  _Window* implementation;

 public:
  Window();
  void init();
  void step();
  void halt();
  bool done();
  void size(int& w, int& h);
};

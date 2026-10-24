#pragma once

#include <memory>

class Assignment2App {
  public:
    Assignment2App();
    ~Assignment2App();

    Assignment2App(const Assignment2App &) = delete;
    Assignment2App &operator=(const Assignment2App &) = delete;

    void draw();

  private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

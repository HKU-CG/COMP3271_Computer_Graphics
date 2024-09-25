#include "MainFrame.h"
#include <memory>

int main() {
    auto main_frame = std::make_unique<MainFrame>();
    main_frame->MainLoop();
}
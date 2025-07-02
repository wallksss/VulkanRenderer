#include "vk_engine.h"
#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>

int main(int argc, char* argv[]) {
  VulkanApplication app;

  try {
    app.init();
    app.run();
    app.cleanup();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

#include "vk_engine.h"
#include <cstdlib>
#include <iostream>
#include <memory>

int main(int argc, char* argv[]) {
  std::unique_ptr<VulkanApplication> app = std::make_unique<VulkanApplication>();

  try {
    app->init();
    app->run();
    app->cleanup();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

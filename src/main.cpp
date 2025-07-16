// ADICIONAR (Substitua o conteúdo de main.cpp inteiro)

#include "vk_engine.h"
#include <cstdlib>
#include <iostream>
#include <memory> // Incluir para std::unique_ptr

int main(int argc, char* argv[]) {
  // Usamos um ponteiro inteligente (unique_ptr) para gerenciar a vida do objeto.
  // O objeto 'app' agora é alocado no HEAP, não na pilha.
  std::unique_ptr<VulkanApplication> app = std::make_unique<VulkanApplication>();

  try {
    // Usamos o operador '->' em vez de '.' para acessar membros através de um ponteiro.
    app->init();
    app->run();
    app->cleanup();
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // O unique_ptr chama o destrutor e libera a memória automaticamente quando 'main' termina.
  return EXIT_SUCCESS;
}
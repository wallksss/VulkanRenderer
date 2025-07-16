#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

struct Vertex {
  glm::vec3 pos;
  glm::vec2 texCoord;
  //glm::vec3 normal;

  // Returns the binding description for a vertex.
  static VkVertexInputBindingDescription getBindingDescription();
  
  // Returns the attribute descriptions for a vertex.
  static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();

  bool operator==(const Vertex& other) const {
    return pos == other.pos && texCoord == other.texCoord;
  }
};

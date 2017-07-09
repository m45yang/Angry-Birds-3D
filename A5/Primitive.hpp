#pragma once

#include <glm/glm.hpp>
#include <string>

class Primitive {
public:
  Primitive(
    const std::string & type,
    glm::vec3 pos,
    glm::vec3 size
  );

  std::string m_type;
  glm::vec3 m_pos;
  glm::vec3 m_size;
};

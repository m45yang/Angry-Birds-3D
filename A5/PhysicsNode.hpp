#pragma once

#include "SceneNode.hpp"

class PhysicsNode : public SceneNode {
public:
  PhysicsNode(
    const std::string & name
  );

  void set_velocity(const glm::vec3 & v);
  void set_gravity(bool g);

  glm::vec3 m_velocity;
  bool m_gravity;
};

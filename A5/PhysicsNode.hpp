#pragma once

#include "SceneNode.hpp"

class PhysicsNode : public SceneNode {
public:
  PhysicsNode(
    const std::string & name
  );

  void set_velocity(const glm::vec3 & v);

  glm::vec3 velocity;
};

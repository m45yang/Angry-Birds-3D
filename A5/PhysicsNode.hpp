#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"

class PhysicsNode : public SceneNode {
public:
  PhysicsNode(
    const std::string & name,
    Primitive *prim
  );
  ~PhysicsNode();

  void translate(const glm::vec3 & amount);
  void set_velocity(const glm::vec3 & v);
  void set_gravity(bool g);

  glm::vec3 m_velocity;
  bool m_gravity;

  Primitive *m_primitive;
};

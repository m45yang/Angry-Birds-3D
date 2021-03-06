#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"
#include "Material.hpp"

class PhysicsNode : public SceneNode {
public:
  PhysicsNode(
    const std::string & name,
    Primitive *prim,
    unsigned int object_type
  );
  ~PhysicsNode();

  void translate(const glm::vec3 & amount);
  void set_velocity(const glm::vec3 & v);
  void set_gravity(bool g);

  Material material;

  glm::vec3 m_velocity;
  bool m_gravity;
  bool m_destroyed;
  ObjectType m_objectType;

  Primitive *m_primitive;
};

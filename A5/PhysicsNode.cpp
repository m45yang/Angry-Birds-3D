#include "PhysicsNode.hpp"

using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
PhysicsNode::PhysicsNode(
    const string & name
)
  : SceneNode(name)
{
  m_nodeType = NodeType::PhysicsNode;
  m_gravity = true;
}

//---------------------------------------------------------------------------------------
void PhysicsNode::set_velocity(const vec3 & v) {
  m_velocity = v;
}

//---------------------------------------------------------------------------------------
void PhysicsNode::set_gravity(bool g) {
  m_gravity = g;
}

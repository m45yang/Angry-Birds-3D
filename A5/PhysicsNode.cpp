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
}

//---------------------------------------------------------------------------------------
void PhysicsNode::set_velocity(const vec3 & v) {
  velocity = v;
}

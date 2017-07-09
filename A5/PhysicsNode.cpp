#include "PhysicsNode.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
PhysicsNode::PhysicsNode(
    const string & name,
    Primitive *prim
)
  : SceneNode( name )
  , m_primitive( prim )
{
  m_nodeType = NodeType::PhysicsNode;
  m_gravity = true;

  scale(prim->m_size);
  translate(prim->m_pos);
}

//---------------------------------------------------------------------------------------
PhysicsNode::~PhysicsNode()
{
  delete m_primitive;
}

//---------------------------------------------------------------------------------------
void PhysicsNode::translate(const vec3 & amount) {
  mat4 t = glm::translate(amount);
  trans = t * trans;

  m_primitive->m_pos.x = trans[3][0];
  m_primitive->m_pos.y = trans[3][1];
  m_primitive->m_pos.z = trans[3][2];
}

//---------------------------------------------------------------------------------------
void PhysicsNode::set_velocity(const vec3 & v) {
  m_velocity = v;
}

//---------------------------------------------------------------------------------------
void PhysicsNode::set_gravity(bool g) {
  m_gravity = g;
}

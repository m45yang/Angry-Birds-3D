#include "AnimationNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <math.h>

using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
AnimationNode::AnimationNode(
    const string & name
)
  : SceneNode( name )
  , m_keyframe1(mat4())
  , m_keyframe2(mat4())
  , m_dt(0.0)
  , m_direction(Direction::Up)
{
  m_nodeType = NodeType::AnimationNode;
}

//---------------------------------------------------------------------------------------
AnimationNode::~AnimationNode()
{
}

//---------------------------------------------------------------------------------------
void AnimationNode::rotateKeyframe(char axis, float angle, unsigned int keyframe) {
  vec3 rot_axis;

  switch (axis) {
    case 'x':
      rot_axis = vec3(1,0,0);
      break;
    case 'y':
      rot_axis = vec3(0,1,0);
          break;
    case 'z':
      rot_axis = vec3(0,0,1);
          break;
    default:
      break;
  }
  mat4 rot_matrix = glm::rotate(degreesToRadians(angle), rot_axis);

  if (keyframe == 1) {
    m_keyframe1 = rot_matrix * m_keyframe1;
  }
  else if (keyframe == 2) {
    m_keyframe2 = rot_matrix * m_keyframe2;
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::scaleKeyframe(const glm::vec3 & amount, unsigned int keyframe) {
  if (keyframe == 1) {
    m_keyframe1 = glm::scale(amount);
  }
  else if (keyframe == 2) {
    m_keyframe2 = glm::scale(amount);
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::translateKeyframe(const glm::vec3& amount, unsigned int keyframe) {
  if (keyframe == 1) {
    m_keyframe1 = glm::translate(amount) * m_keyframe1;
  }
  else if (keyframe == 2) {
    m_keyframe2 = glm::translate(amount) * m_keyframe2;
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::computeTrans()
{
  mat4 k1(
    vec4(m_dt*m_keyframe1[0][0], m_dt*m_keyframe1[0][1], m_dt*m_keyframe1[0][2], m_keyframe1[0][3]),
    vec4(m_dt*m_keyframe1[1][0], m_dt*m_keyframe1[1][1], m_dt*m_keyframe1[1][2], m_keyframe1[1][3]),
    vec4(m_dt*m_keyframe1[2][0], m_dt*m_keyframe1[2][1], m_dt*m_keyframe1[2][2], m_keyframe1[2][3]),
    vec4(m_dt*m_keyframe1[3][0], m_dt*m_keyframe1[3][1], m_dt*m_keyframe1[3][2], 0.5*m_keyframe1[3][3])
  );

  mat4 k2(
    vec4((1-m_dt)*m_keyframe2[0][0], (1-m_dt)*m_keyframe2[0][1], (1-m_dt)*m_keyframe2[0][2], m_keyframe2[0][3]),
    vec4((1-m_dt)*m_keyframe2[1][0], (1-m_dt)*m_keyframe2[1][1], (1-m_dt)*m_keyframe2[1][2], m_keyframe2[1][3]),
    vec4((1-m_dt)*m_keyframe2[2][0], (1-m_dt)*m_keyframe2[2][1], (1-m_dt)*m_keyframe2[2][2], m_keyframe2[2][3]),
    vec4((1-m_dt)*m_keyframe2[3][0], (1-m_dt)*m_keyframe2[3][1], (1-m_dt)*m_keyframe2[3][2], 0.5*m_keyframe2[3][3])
  );

  trans = k1 + k2;
}

//---------------------------------------------------------------------------------------
void AnimationNode::updateKeyframe(double dt) {
  dt = fmod(dt, 1);

  if (m_direction == Direction::Up) {
    if (m_dt + dt > 1.0) {
      m_dt = 1.0;
      m_direction = Direction::Down;
    }
    else {
      m_dt = m_dt + dt;
    }
  }
  else if (m_direction == Direction::Down) {
    if (m_dt - dt < 0.0) {
      m_dt = 0.0;
      m_direction = Direction::Up;
    }
    else {
      m_dt = m_dt - dt;
    }
  }

  computeTrans();
}

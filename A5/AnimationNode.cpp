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
  , m_current_keyframe(1)
{
  m_nodeType = NodeType::AnimationNode;
}

//---------------------------------------------------------------------------------------
AnimationNode::~AnimationNode()
{
}

//---------------------------------------------------------------------------------------
void AnimationNode::addKeyframe()
{
  m_keyframes.push_back(mat4());
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

  if (keyframe <= m_keyframes.size()) {
    m_keyframes[keyframe] = glm::rotate(degreesToRadians(angle), rot_axis) * m_keyframes[keyframe];
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::scaleKeyframe(const glm::vec3 & amount, unsigned int keyframe) {
  if (keyframe <= m_keyframes.size()) {
    m_keyframes[keyframe] = glm::scale(amount) * m_keyframes[keyframe];
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::translateKeyframe(const glm::vec3& amount, unsigned int keyframe) {
if (keyframe <= m_keyframes.size()) {
    m_keyframes[keyframe] = glm::translate(amount) * m_keyframes[keyframe];
  }
}

//---------------------------------------------------------------------------------------
void AnimationNode::computeTrans()
{
  mat4 current_keyframe = m_keyframes[m_current_keyframe];
  mat4 previous_keyframe = m_keyframes[m_current_keyframe-1];

  mat4 k1(
    vec4(m_dt*current_keyframe[0][0], m_dt*current_keyframe[0][1], m_dt*current_keyframe[0][2], current_keyframe[0][3]),
    vec4(m_dt*current_keyframe[1][0], m_dt*current_keyframe[1][1], m_dt*current_keyframe[1][2], current_keyframe[1][3]),
    vec4(m_dt*current_keyframe[2][0], m_dt*current_keyframe[2][1], m_dt*current_keyframe[2][2], current_keyframe[2][3]),
    vec4(m_dt*current_keyframe[3][0], m_dt*current_keyframe[3][1], m_dt*current_keyframe[3][2], 0.5*current_keyframe[3][3])
  );

  mat4 k2(
    vec4((1-m_dt)*previous_keyframe[0][0], (1-m_dt)*previous_keyframe[0][1], (1-m_dt)*previous_keyframe[0][2], (1-m_dt)*previous_keyframe[0][3]),
    vec4((1-m_dt)*previous_keyframe[1][0], (1-m_dt)*previous_keyframe[1][1], (1-m_dt)*previous_keyframe[1][2], (1-m_dt)*previous_keyframe[1][3]),
    vec4((1-m_dt)*previous_keyframe[2][0], (1-m_dt)*previous_keyframe[2][1], (1-m_dt)*previous_keyframe[2][2], (1-m_dt)*previous_keyframe[2][3]),
    vec4((1-m_dt)*previous_keyframe[3][0], (1-m_dt)*previous_keyframe[3][1], (1-m_dt)*previous_keyframe[3][2], 0.5*previous_keyframe[3][3])
  );

  trans = k1 + k2;
}

//---------------------------------------------------------------------------------------
void AnimationNode::updateKeyframe(double dt) {
  if (m_dt + dt > 1.0) {
    m_dt = m_dt + dt - 1.0;
    if (m_current_keyframe >= m_keyframes.size()-1) {
      m_current_keyframe = 1;
    }
    else {
      m_current_keyframe++;
    }
  }
  else {
    m_dt = m_dt + dt;
  }

  computeTrans();
}

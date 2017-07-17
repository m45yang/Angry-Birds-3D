#pragma once

#include "SceneNode.hpp"
#include "Primitive.hpp"

class AnimationNode : public SceneNode {
public:
  AnimationNode(
    const std::string & name,
    Primitive *prim,
    unsigned int object_type
  );
  ~AnimationNode();

  void addKeyframe();
  void rotateKeyframe(char axis, float angle, unsigned int keyframe);
  void scaleKeyframe(const glm::vec3 & amount, unsigned int keyframe);
  void translateKeyframe(const glm::vec3& amount, unsigned int keyframe);
  void computeTrans();
  void updateKeyframe(double dt);

  glm::mat4 m_keyframe_trans;
  std::vector<glm::mat4> m_keyframes;
  unsigned int m_current_keyframe;

  ObjectType m_objectType;
  Primitive *m_primitive;
  bool m_destroyed;
  double m_dt;
};

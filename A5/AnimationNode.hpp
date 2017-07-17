#pragma once

#include "SceneNode.hpp"


class AnimationNode : public SceneNode {
public:
  AnimationNode(
    const std::string & name
  );
  ~AnimationNode();

  void addKeyframe();
  void rotateKeyframe(char axis, float angle, unsigned int keyframe);
  void scaleKeyframe(const glm::vec3 & amount, unsigned int keyframe);
  void translateKeyframe(const glm::vec3& amount, unsigned int keyframe);
  void computeTrans();
  void updateKeyframe(double dt);

  glm::mat4 m_keyframe1;
  glm::mat4 m_keyframe2;

  std::vector<glm::mat4> m_keyframes;
  unsigned int m_current_keyframe;

  double m_dt;
};

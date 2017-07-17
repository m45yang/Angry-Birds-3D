#pragma once

#include "SceneNode.hpp"

enum class Direction {
    Up,
    Down
};


class AnimationNode : public SceneNode {
public:
  AnimationNode(
    const std::string & name
  );
  ~AnimationNode();

  void rotateKeyframe(char axis, float angle, unsigned int keyframe);
  void scaleKeyframe(const glm::vec3 & amount, unsigned int keyframe);
  void translateKeyframe(const glm::vec3& amount, unsigned int keyframe);
  void computeTrans();
  void updateKeyframe(double dt);

  glm::mat4 m_keyframe1;
  glm::mat4 m_keyframe2;

  double m_dt;
  Direction m_direction;
};

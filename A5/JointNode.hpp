#pragma once

#include "SceneNode.hpp"
#include <map>

class JointNode : public SceneNode {
public:
  JointNode(const std::string & name);
  virtual ~JointNode();

  void set_joint_x(double min, double init, double max);
  void set_joint_y(double min, double init, double max);

  struct JointRange {
    double min, init, max;
  };


  JointRange m_joint_x, m_joint_y;

  static std::map< unsigned int, double > jointNodeX;
  static std::map< unsigned int, double > jointNodeY;
};

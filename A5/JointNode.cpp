#include "JointNode.hpp"

using namespace std;

map<unsigned int, double > JointNode::jointNodeX;
map<unsigned int, double > JointNode::jointNodeY;

//---------------------------------------------------------------------------------------
JointNode::JointNode(const std::string& name)
  : SceneNode(name)
{
  m_nodeType = NodeType::JointNode;
}

//---------------------------------------------------------------------------------------
JointNode::~JointNode() {

}
 //---------------------------------------------------------------------------------------
void JointNode::set_joint_x(double min, double init, double max) {
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;

  jointNodeX[m_nodeId] = init;
}

//---------------------------------------------------------------------------------------
void JointNode::set_joint_y(double min, double init, double max) {
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;

  jointNodeY[m_nodeId] = init;
}

#include "GeometryNode.hpp"

//---------------------------------------------------------------------------------------
GeometryNode::GeometryNode(
    const std::string & meshId,
    const std::string & name,
    const unsigned int & texture
)
  : SceneNode(name),
    meshId(meshId),
    texture(texture)
{
  m_nodeType = NodeType::GeometryNode;
}

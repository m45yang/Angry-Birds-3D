#pragma once

#include "SceneNode.hpp"

class GeometryNode : public SceneNode {
public:
  GeometryNode(
    const std::string & meshId,
    const std::string & name,
    const unsigned int & texture
  );

  Material material;

  // Mesh Identifier. This must correspond to an object name of
  // a loaded .obj file.
  std::string meshId;

  // Texture identifier. This must correspond to a texture id
  // loaded in A5.cpp.
  unsigned int texture;
};

#include "Primitive.hpp"

Primitive::~Primitive()
{
}

Sphere::~Sphere()
{
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
}

glm::vec3 NonhierSphere::getPosition()
{
  return m_pos;
}

double NonhierSphere::getRadius()
{
  return m_radius;
}

NonhierBox::~NonhierBox()
{
}

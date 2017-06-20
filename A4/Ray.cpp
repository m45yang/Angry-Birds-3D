#include <glm/ext.hpp>

#include "Ray.hpp"

using namespace std;
using namespace glm;

//---------------------------------------------------------------------------------------
Ray::Ray(
  const glm::vec3 &origin, const glm::vec3 &direction )
  :
  origin( origin ),
  direction( direction ),
  tMin( 0.1 ),
  tMax( 100 )
{}

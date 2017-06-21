#include <glm/glm.hpp>

class Ray {
public:
  Ray( const glm::vec3 &origin, const glm::vec3 &direction );

  glm::vec3 origin;
  glm::vec3 direction;
};

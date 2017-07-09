#include "Primitive.hpp"

using namespace std;

//---------------------------------------------------------------------------------------
Primitive::Primitive(
    const string & type,
    glm::vec3 pos,
    glm::vec3 size
)
  : m_type( type )
  , m_pos( pos )
  , m_size( size )
{
}

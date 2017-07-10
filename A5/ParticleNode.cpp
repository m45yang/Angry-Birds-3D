#include "ParticleNode.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
ParticleNode::ParticleNode(
  const string & name
)
  : SceneNode( name )
  , m_lastUsedParticle( 0 )
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( "Assets/particleNode_VertexShader.vs" );
  m_shader.attachFragmentShader( "Assets/particleNode_FragmentShader.fs" );
  m_shader.link();

  glGenVertexArrays(1, &m_vao);
  enableVertexShaderInputSlots();
  uploadVertexDataToVbos();
  mapVboDataToVertexShaderInputLocations();
}

//---------------------------------------------------------------------------------------
ParticleNode::~ParticleNode() {

}

//----------------------------------------------------------------------------------------
void ParticleNode::enableVertexShaderInputSlots()
{
  {
    glBindVertexArray(m_vao);

    m_positionAttribLocation = m_shader.getAttribLocation("position");
    glEnableVertexAttribArray(m_positionAttribLocation);

    CHECK_GL_ERRORS;
  }

  glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void ParticleNode::uploadVertexDataToVbos() {
  float particle_quad[] = {
    0.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,

    0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 0.0f,
    1.0f, 0.0f, 0.0f
  };

  glGenBuffers(1, &m_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void ParticleNode::mapVboDataToVertexShaderInputLocations() {
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

void ParticleNode::Update(float dt, vec3 offset)
{
    // Add new particles
    unsigned int unusedParticle = firstUnusedParticle();
    respawnParticle(m_particles[unusedParticle], offset);

    // Update all particles
    for (GLuint i = 0; i < m_max_particles; ++i)
    {
        Particle p = m_particles[i];
        p.life -= dt; // reduce life
        if (p.life > 0.0f) { // particle is alive, thus update
            p.position -= p.velocity * dt;
            p.color.a -= dt * 2.5;
        }
    }
}

//----------------------------------------------------------------------------------------
unsigned int ParticleNode::firstUnusedParticle() {
  // First search from last used particle, this will usually return almost instantly
  for (int i = m_lastUsedParticle; i < m_max_particles; ++i){
      if (m_particles[i].life <= 0.0f){
          m_lastUsedParticle = i;
          return i;
      }
  }
  // Otherwise, do a linear search
  for (int i = 0; i < m_lastUsedParticle; ++i){
      if (m_particles[i].life <= 0.0f){
          m_lastUsedParticle = i;
          return i;
      }
  }
  // All particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
  m_lastUsedParticle = 0;
  return 0;
}

//----------------------------------------------------------------------------------------
void ParticleNode::respawnParticle(Particle &particle, vec3 offset)
{
  vec3 position(trans[3][0], trans[3][1], trans[3][2]);
  float random = ((rand() % 100) - 50) / 10.0f;
  float rColor = 0.5 + ((rand() % 100) / 100.0f);

  particle.position = position + random + offset;
  particle.color = glm::vec4(rColor, rColor, rColor, 1.0f);
  particle.life = 1.0f;
  particle.velocity = m_velocity * 0.1f;
}


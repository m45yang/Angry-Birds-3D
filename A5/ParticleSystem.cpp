#include "ParticleSystem.hpp"
#include "cs488-framework/GlErrorCheck.hpp"
#include <glm/gtx/io.hpp>


using namespace glm;
using namespace std;

//---------------------------------------------------------------------------------------
ParticleSystem::ParticleSystem(
  double life
)
  : m_lastUsedParticle( 0 )
  , m_life( life )
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( "Assets/particleNode_VertexShader.vs" );
  m_shader.attachFragmentShader( "Assets/particleNode_FragmentShader.fs" );
  m_shader.link();

  glGenVertexArrays(1, &m_vao);
  enableVertexShaderInputSlots();
  uploadVertexDataToVbos();
  mapVboDataToVertexShaderInputLocations();

  // Create m_max_particles particle instances
  for (int i=0; i<m_max_particles; ++i) {
    m_particles.push_back(Particle());
  }
}

//---------------------------------------------------------------------------------------
ParticleSystem::~ParticleSystem() {
  delete[] &m_particles[0];
}

//----------------------------------------------------------------------------------------
void ParticleSystem::enableVertexShaderInputSlots()
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
void ParticleSystem::uploadVertexDataToVbos() {
  float particle_quad[] = {
    0.0f, 0.25f, 0.0f,
    0.25f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f,

    0.0f, 0.25f, 0.0f,
    0.25f, 0.25f, 0.0f,
    0.25f, 0.0f, 0.0f
  };

  glGenBuffers(1, &m_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void ParticleSystem::mapVboDataToVertexShaderInputLocations() {
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

void ParticleSystem::update(double dt)
{
    double random1, random2, random3;
    // Add new particles
    if (m_life > 0.0) {
      for (int i=0; i<10; i++) {
        unsigned int unusedParticle = firstUnusedParticle();
        respawnParticle(m_particles[unusedParticle]);
      }
    }

    // Update all particles
    for (vector<Particle>::iterator particle=m_particles.begin(); particle!=m_particles.end(); particle++)
    {
        particle->life -= dt; // reduce life

        random1 = (rand() % 100);
        random2 = (rand() % 100);
        random3 = (rand() % 100);

        // if (it - m_particles.begin() == 1) {
        //   cout << particle.position << endl;
        //   cout << particle.life << endl;
        // }
        if (particle->life > 0.0f) { // particle is alive, thus update
            particle->position.x += particle->velocity.x * random1 * dt;
            particle->position.y += particle->velocity.y * random2 * dt;
            particle->position.z += particle->velocity.z * random3 * dt;
            // particleNode.color.a -= dt * 2.5;
            const vec3 velocity(
              particle->velocity.x,
              particle->velocity.y + (-9.81 * dt),
              particle->velocity.z
            );

            particle->velocity = velocity;
        }
    }

    m_life -= dt;
}

//----------------------------------------------------------------------------------------
unsigned int ParticleSystem::firstUnusedParticle() {
  // First search from last used particle, this will usually return almost instantly
  for (int i=m_lastUsedParticle; i<m_max_particles; ++i){
      if (m_particles[i].life <= 0.0f){
          m_lastUsedParticle = i;
          return i;
      }
  }
  // Otherwise, do a linear search
  for (int i=0; i<m_lastUsedParticle; ++i){
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
void ParticleSystem::respawnParticle(Particle &particle)
{
  double random1 = ((rand() % 100) - 50) / 100.0;
  double random2 = ((rand() % 100) - 50) / 100.0;
  double random3 = ((rand() % 100) - 50) / 100.0;

  particle.position.x = m_position.x + random1;
  particle.position.y = m_position.y + random2;
  particle.position.z = m_position.z + random3;

  random1 = ((rand() % 100) - 50) / 100.0;
  random2 = ((rand() % 100) - 50) / 100.0;
  random3 = ((rand() % 100) - 50) / 100.0;

  particle.velocity.x = m_velocity.x + random1;
  particle.velocity.y = m_velocity.y + random2;
  particle.velocity.z = m_velocity.z + random3;

  float rColor = 0.5 + ((rand() % 100) / 100.0f);
  // particleNode.color = glm::vec4(rColor, rColor, rColor, 1.0f);

  particle.life = m_particle_life;
}


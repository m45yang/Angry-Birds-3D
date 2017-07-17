#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <string>
#include <vector>

struct Particle {
    glm::vec3 position, velocity;
    GLfloat life;

    Particle()
      : position(0.0f), velocity(0.0f), life(0.0f) { }
};

class ParticleSystem {
public:
  ParticleSystem(double life, glm::vec4 color);
  ~ParticleSystem();

  void enableVertexShaderInputSlots();
  void uploadVertexDataToVbos();
  void mapVboDataToVertexShaderInputLocations();
  void update(double dt);
  unsigned int firstUnusedParticle();
  void respawnParticle(Particle &particle);

  //-- GL resources for particle data
  GLuint m_vbo;
  GLuint m_vao;
  GLint m_positionAttribLocation;
  ShaderProgram m_shader;

  // Dynamics data
  glm::vec3 m_velocity;
  glm::vec3 m_position;

  glm::vec4 m_color;

  double m_life;
  unsigned int m_lastUsedParticle;
  const static unsigned int m_max_particles = 150;
  const static unsigned int m_particle_life = 2.0;
  std::vector<Particle> m_particles;
};

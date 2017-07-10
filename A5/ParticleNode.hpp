#pragma once

#include "cs488-framework/ShaderProgram.hpp"
#include "SceneNode.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <string>

struct Particle {
    glm::vec3 position, velocity;
    glm::vec4 color;
    float life;

    Particle()
      : position(0.0f), velocity(0.0f), color(1.0f), life(0.0f) { }
};

class ParticleNode : public SceneNode {
public:
  ParticleNode(const std::string & name);
  ~ParticleNode();

  void enableVertexShaderInputSlots();
  void uploadVertexDataToVbos();
  void mapVboDataToVertexShaderInputLocations();
  void Update(float dt, glm::vec3 offset);
  unsigned int firstUnusedParticle();
  void respawnParticle(Particle &particle, glm::vec3 offset);

  //-- GL resources for particle data
  GLuint m_vbo;
  GLuint m_vao;
  GLint m_positionAttribLocation;
  ShaderProgram m_shader;

  // Dynamics data
  glm::vec3 m_velocity;

  unsigned int m_lastUsedParticle;
  const static unsigned int m_max_particles = 1000;
  Particle m_particles[m_max_particles];
};

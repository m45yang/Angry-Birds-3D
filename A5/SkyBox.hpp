#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <vector>

class SkyBox {
public:
  SkyBox();
  virtual ~SkyBox();

  void renderSkyBox();
  void loadCubeMap(std::vector<std::string> faces);

  //-- GL resources for skybox data
  GLuint m_vbo_skybox;
  GLuint m_vao_skybox;
  GLint m_skybox_positionAttribLocation;
  ShaderProgram m_shader_skybox;

  unsigned int m_cubeTextureID;
};

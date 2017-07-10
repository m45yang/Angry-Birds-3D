#pragma once

#include "cs488-framework/ShaderProgram.hpp"

#include <iostream>
#include <glm/glm.hpp>
#include <vector>

class SkyBox {
public:
  SkyBox();
  virtual ~SkyBox();

  void loadCubeMap(std::vector<std::string> faces);
  void enableVertexShaderInputSlots();
  void uploadVertexDataToVbos();
  void mapVboDataToVertexShaderInputLocations();

  //-- GL resources for skybox data
  GLuint m_vbo;
  GLuint m_vao;
  GLint m_positionAttribLocation;
  ShaderProgram m_shader;

  unsigned int m_cubeTextureID;

private:
  std::vector<float> m_skyboxVertices {
    -50.0f,  50.0f, -50.0f,
    -50.0f, -50.0f, -50.0f,
    50.0f, -50.0f, -50.0f,
     50.0f, -50.0f, -50.0f,
     50.0f,  50.0f, -50.0f,
    -50.0f,  50.0f, -50.0f,

    -50.0f, -50.0f,  50.0f,
    -50.0f, -50.0f, -50.0f,
    -50.0f,  50.0f, -50.0f,
    -50.0f,  50.0f, -50.0f,
    -50.0f,  50.0f,  50.0f,
    -50.0f, -50.0f,  50.0f,

     50.0f, -50.0f, -50.0f,
     50.0f, -50.0f,  50.0f,
     50.0f,  50.0f,  50.0f,
     50.0f,  50.0f,  50.0f,
     50.0f,  50.0f, -50.0f,
     50.0f, -50.0f, -50.0f,

    -50.0f, -50.0f,  50.0f,
    -50.0f,  50.0f,  50.0f,
     50.0f,  50.0f,  50.0f,
     50.0f,  50.0f,  50.0f,
     50.0f, -50.0f,  50.0f,
    -50.0f, -50.0f,  50.0f,

    -50.0f,  50.0f, -50.0f,
     50.0f,  50.0f, -50.0f,
     50.0f,  50.0f,  50.0f,
     50.0f,  50.0f,  50.0f,
    -50.0f,  50.0f,  50.0f,
    -50.0f,  50.0f, -50.0f,

    -50.0f, -50.0f, -50.0f,
    -50.0f, -50.0f,  50.0f,
     50.0f, -50.0f, -50.0f,
     50.0f, -50.0f, -50.0f,
    -50.0f, -50.0f,  50.0f,
     50.0f, -50.0f,  50.0f
  };
};

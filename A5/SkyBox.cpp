#include "SkyBox.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include "stb_image.h"

using namespace std;
using namespace glm;

//---------------------------------------------------------------------------------------
SkyBox::SkyBox() {
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( "Assets/skybox_VertexShader.vs" );
  m_shader.attachFragmentShader( "Assets/skybox_FragmentShader.fs" );
  m_shader.link();

  glGenVertexArrays(1, &m_vao);
  enableVertexShaderInputSlots();
  uploadVertexDataToVbos();
  mapVboDataToVertexShaderInputLocations();
}

//---------------------------------------------------------------------------------------
SkyBox::~SkyBox() {

}

//----------------------------------------------------------------------------------------
void SkyBox::loadCubeMap(vector<string> faces)
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            cout << "Cubemap texture failed to load at path: " << faces[i] << endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    m_cubeTextureID = texture;
}

//----------------------------------------------------------------------------------------
void SkyBox::enableVertexShaderInputSlots()
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
void SkyBox::uploadVertexDataToVbos() {
  glGenBuffers(1, &m_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

  glBufferData(GL_ARRAY_BUFFER, m_skyboxVertices.size()*sizeof(float), &m_skyboxVertices[0], GL_STATIC_DRAW);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void SkyBox::mapVboDataToVertexShaderInputLocations() {
  glBindVertexArray(m_vao);

  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

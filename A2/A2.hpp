#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"

#include <glm/glm.hpp>

#include <vector>
#include <bitset>

// Set a global maximum number of vertices in order to pre-allocate VBO data
// in one shot, rather than reallocating each frame.
const GLsizei kMaxVertices = 1000;


// Convenience class for storing vertex data in CPU memory.
// Data should be copied over to GPU memory via VBO storage before rendering.
class VertexData {
public:
  VertexData();

  std::vector<glm::vec2> positions;
  std::vector<glm::vec3> colours;
  GLuint index;
  GLsizei numVertices;
};


class A2 : public CS488Window {
public:
  A2();
  virtual ~A2();

protected:
  virtual void init() override;
  virtual void appLogic() override;
  virtual void guiLogic() override;
  virtual void draw() override;
  virtual void cleanup() override;

  virtual bool cursorEnterWindowEvent(int entered) override;
  virtual bool mouseMoveEvent(double xPos, double yPos) override;
  virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
  virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
  virtual bool windowResizeEvent(int width, int height) override;
  virtual bool keyInputEvent(int key, int action, int mods) override;

  void createShaderProgram();
  void enableVertexAttribIndices();
  void generateVertexBuffers();
  void mapVboDataToVertexAttributeLocation();
  void uploadVertexDataToVbos();
  void initializeModelCoordinates();
  void initializeCoordinateFrames();
  void initializeTransformationMatrices();
  void applyModelTransformation();
  void applyViewingTransformation();
  void applyProjectionTransformation();
  void buildAndClipLines();
  void performClipping(std::vector< std::pair< glm::vec4, glm::vec4> >, std::vector<glm::vec2>*);
  std::bitset<6> generateOutCode(glm::vec4);
  glm::vec2 normalizeVertex(glm::vec4);

  void initLineData();

  void setLineColour(const glm::vec3 & colour);

  void drawLine (
      const glm::vec2 & v0,
      const glm::vec2 & v1
  );

  ShaderProgram m_shader;

  GLuint m_vao;            // Vertex Array Object
  GLuint m_vbo_positions;  // Vertex Buffer Object
  GLuint m_vbo_colours;    // Vertex Buffer Object

  // Matrices controlling the camera and projection.
  glm::mat4 t_proj;
  glm::mat4 t_view;
  glm::mat4 t_model_cube;
  glm::mat4 t_model_cube_gnomon;
  glm::mat4 t_model_scale;
  glm::mat4 t_model_rotation;
  glm::mat4 t_model_translation;

  VertexData m_vertexData;

  glm::vec3 m_currentLineColour;
  std::vector<glm::vec4> cube_model_coordinates;
  std::vector<glm::vec4> cube_world_coordinates;
  std::vector<glm::vec4> cube_view_coordinates;
  std::vector<glm::vec4> cube_clip_coordinates;
  std::vector<glm::vec2> cube_normalized_device_coordinates;

  std::vector<glm::vec4> cube_gnomon_model_coordinates;
  std::vector<glm::vec4> cube_gnomon_world_coordinates;
  std::vector<glm::vec4> cube_gnomon_view_coordinates;
  std::vector<glm::vec4> cube_gnomon_clip_coordinates;
  std::vector<glm::vec2> cube_gnomon_normalized_device_coordinates;

  std::vector<glm::vec4> world_gnomon_model_coordinates;
  std::vector<glm::vec4> world_gnomon_world_coordinates;
  std::vector<glm::vec4> world_gnomon_view_coordinates;
  std::vector<glm::vec2> world_gnomon_normalized_device_coordinates;

  std::vector<glm::vec4> f_model;
  std::vector<glm::vec4> f_world;
  std::vector<glm::vec4> f_view;

  std::vector< std::pair< glm::vec4, glm::vec4 > > cube_lines;
  std::vector< std::pair< glm::vec4, glm::vec4 > > cube_gnomon_lines;

  bool keys[1024];
  bool is_changing_viewport;
  int current_mode;
  float mouse_x_pos;
  float mouse_y_pos;
  float viewport_xl;
  float viewport_yb;
  float viewport_xr;
  float viewport_yt;
  float left_bound;
  float bottom_bound;;
  float width_ratio;
  float height_ratio;
  float near;
  float far;
  float fov;
};

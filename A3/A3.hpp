#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"
#include "trackball.hpp"

#include <glm/glm.hpp>
#include <map>
#include <memory>

struct LightSource {
  glm::vec3 position;
  glm::vec3 rgbIntensity;
};


class A3 : public CS488Window {
public:
  A3(const std::string & luaSceneFile);
  virtual ~A3();

protected:
  virtual void init() override;
  virtual void appLogic() override;
  virtual void guiLogic() override;
  virtual void draw() override;
  virtual void cleanup() override;

  //-- Virtual callback methods
  virtual bool cursorEnterWindowEvent(int entered) override;
  virtual bool mouseMoveEvent(double xPos, double yPos) override;
  virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
  virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
  virtual bool windowResizeEvent(int width, int height) override;
  virtual bool keyInputEvent(int key, int action, int mods) override;

  //-- One time initialization methods:
  void processLuaSceneFile(const std::string & filename);
  void createShaderProgram();
  void enableVertexShaderInputSlots();
  void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
  void mapVboDataToVertexShaderInputLocations();
  void initModelMatrices();
  void initViewMatrix();
  void initLightSources();
  void clearJointsAngleStack();
  void pushJointsAngleStack();
  void moveJointsAngleStackIndex(int amount);

  void initPerspectiveMatrix();
  void uploadCommonSceneUniforms();
  void renderSceneGraph(const SceneNode &node);
  void renderNode(const SceneNode &node);
  void renderArcCircle();

  glm::mat4 m_perpsective;
  glm::mat4 m_view;
  glm::mat4 m_model_rotation;
  glm::mat4 m_model_translation;

  LightSource m_light;

  //-- GL resources for mesh geometry data:
  GLuint m_vao_meshData;
  GLuint m_vbo_vertexPositions;
  GLuint m_vbo_vertexNormals;
  GLint m_positionAttribLocation;
  GLint m_normalAttribLocation;
  ShaderProgram m_shader;

  //-- GL resources for trackball circle geometry:
  GLuint m_vbo_arcCircle;
  GLuint m_vao_arcCircle;
  GLint m_arc_positionAttribLocation;
  ShaderProgram m_shader_arcCircle;

  // -- GL resources for picking geometry:
  GLuint m_vao_pickingMeshData;
  GLuint m_vbo_pickingVertexPositions;
  GLint m_picking_positionAttribLocation;
  ShaderProgram m_shader_picking;

  // BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
  // object. Each BatchInfo object contains an index offset and the number of indices
  // required to render the mesh with identifier MeshId.
  BatchInfoMap m_batchInfoMap;

  std::string m_luaSceneFile;

  std::shared_ptr<SceneNode> m_rootNode;


  bool cull_front;
  bool cull_back;
  bool do_picking;
  bool draw_trackball_circle;
  bool show_trackball_circle;
  bool z_buffer;
  bool keys[1024];
  float mouse_x_pos;
  float mouse_y_pos;
  float trackball_draw_start_x;
  float trackball_draw_start_y;
  float trackball_circle_size;
  int current_mode;
  int joints_angle_stack_index;
  unsigned int selected;

private:
  static std::stack<glm::mat4> matrixStack;

  // A stack containing all joint rotations in order
  static std::vector< std::map < unsigned int, std::pair< double, double > > > jointsAngleStack;
};

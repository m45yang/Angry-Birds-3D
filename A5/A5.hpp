#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "GeometryNode.hpp"
#include "SceneNode.hpp"
#include "PhysicsNode.hpp"
#include "SkyBox.hpp"

#include <glm/glm.hpp>
#include <map>
#include <memory>
#include <vector>

struct LightSource {
  glm::vec3 position;
  glm::vec3 rgbIntensity;
};


class A5 : public CS488Window {
public:
  A5(const std::string & luaSceneFile);
  virtual ~A5();

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
  void initViewMatrix();
  void initLightSources();
  void initPerspectiveMatrix();
  void getPhysicsNodes(SceneNode &node);
  void getBirdNodes(SceneNode &node);

  void uploadCommonSceneUniforms();
  void updateShaderUniforms(const GeometryNode & node);
  void updateTransformations(double dt);
  bool checkCollision(PhysicsNode *physicsNode);
  void renderSceneGraph(const SceneNode &node);
  void renderNode(const SceneNode &node);
  void renderSkyBox();
  void loadTexture(const char* textureFilePath);

  glm::mat4 m_perspective;
  glm::mat4 m_view;

  LightSource m_light;

  //-- GL resources for mesh geometry data:
  GLuint m_vao_meshData;
  GLuint m_vbo_vertexPositions;
  GLuint m_vbo_vertexNormals;
  GLint m_positionAttribLocation;
  GLint m_normalAttribLocation;
  ShaderProgram m_shader;

  // BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
  // object. Each BatchInfo object contains an index offset and the number of indices
  // required to render the mesh with identifier MeshId.
  BatchInfoMap m_batchInfoMap;

  std::string m_luaSceneFile;

  std::shared_ptr<SceneNode> m_rootNode;

  std::list<PhysicsNode*> m_physicsNodes;
  std::vector<PhysicsNode*> m_birdNodes;

  int m_current_mode;
  unsigned int m_current_bird;
  bool m_keys[1024];
  float m_mouse_x_pos;
  float m_mouse_y_pos;
  float x_velocity;
  float y_velocity;
  float z_velocity;
  unsigned int m_num_textures;
  std::shared_ptr<SkyBox> m_skybox;
  clock_t old_time;
  clock_t new_time;

private:
  static std::stack<glm::mat4> matrixStack;
};

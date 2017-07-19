#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "irrKlang.h"

#include "AnimationNode.hpp"
#include "GeometryNode.hpp"
#include "SceneNode.hpp"
#include "ParticleSystem.hpp"
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

enum class TextureType {
  JPG,
  PNG
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
  void initPerspectiveMatrix();
  void initViewMatrix();
  void initLightPerspectiveMatrix();
  void initLightViewMatrix();
  void initLightSources();
  void getPhysicsNodes(SceneNode &node);
  void getAnimationNodes(SceneNode &node);
  void getBirdNode(SceneNode &node);
  void initDepthMap();

  void updateCamera();
  void uploadCommonSceneUniforms();
  void uploadDepthMapSceneUniforms();
  void updateShaderUniforms(const GeometryNode & node);
  void updateDepthMapShaderUniforms(const GeometryNode & node);
  void updateParticleSystems(double dt);
  void updatePhysicsNodes(double dt);
  void updateAnimationNodes(double dt);
  bool checkCollision(PhysicsNode *physicsNode);
  void destroyPhysicsNode(PhysicsNode *physicsNode);
  void destroyAnimationNode(AnimationNode *animationNode);
  void renderSceneGraph(const SceneNode &node);
  void renderDepthMap(const SceneNode &node);
  void renderNode(const SceneNode &node, unsigned int type);
  void renderParticles();
  void renderSkyBox();
  void loadTexture(const char* textureFilePath, TextureType type);

  glm::mat4 m_perspective;
  glm::mat4 m_view;
  glm::mat4 m_lightPerspective;
  glm::mat4 m_lightView;

  LightSource m_light;

  //-- GL resources for mesh geometry data:
  GLuint m_vao_meshData;
  GLuint m_vbo_vertexPositions;
  GLuint m_vbo_vertexNormals;
  GLint m_positionAttribLocation;
  GLint m_normalAttribLocation;
  ShaderProgram m_shader;

  //-- GL resources for depth map data:
  GLuint m_fbo_depthMap;
  GLuint m_depthMapTexture;
  ShaderProgram m_depthMapShader;

  //-- GL resources for uv coordinates of a cube
  GLuint m_vbo_uvCube;
  GLuint m_vao_uvCube;
  GLuint m_cubeTexCoordsAttribLocation;

  // -- Resources for sound
  irrklang::ISoundEngine *m_themeSoundEngine;
  irrklang::ISoundEngine *m_collisionSoundEngine;

  // BatchInfoMap is an associative container that maps a unique MeshId to a BatchInfo
  // object. Each BatchInfo object contains an index offset and the number of indices
  // required to render the mesh with identifier MeshId.
  BatchInfoMap m_batchInfoMap;

  std::string m_luaSceneFile;

  std::shared_ptr<SceneNode> m_rootNode;

  std::list<ParticleSystem*> m_particleSystems;
  std::list<PhysicsNode*> m_physicsNodes;
  std::list<AnimationNode*> m_animationNodes;
  PhysicsNode *m_birdNode;
  glm::mat4 m_birdNodeOriginalTrans;

  bool m_keys[1024];
  float m_mouse_x_pos;
  float m_mouse_y_pos;
  float x_angle;
  float y_angle;
  float power;
  float power_change;
  unsigned int m_num_textures;
  std::shared_ptr<SkyBox> m_skybox;

private:
  static std::stack<glm::mat4> matrixStack;
};

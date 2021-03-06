#include "A5.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace irrklang;
using namespace glm;

static bool show_gui = true;
stack<mat4> A5::matrixStack;

const string flyingSounds[5] = {
  "Assets/sounds/flying1.wav",
  "Assets/sounds/flying2.wav",
  "Assets/sounds/flying3.wav",
  "Assets/sounds/flying4.wav",
  "Assets/sounds/flying5.wav"
};

const float m_uvCube[72] = {
  // Left face top left
  0.0f, 1.0f,
  1.0f, 1.0f,
  1.0f, 0.0f,

  // Back face top left
  0.0f, 1.0f,
  1.0f, 1.0f,
  1.0f, 0.0f,

  // Right face top left
  0.0f, 1.0f,
  1.0f, 1.0f,
  1.0f, 0.0f,

  // Front face top left
  0.0f, 1.0f,
  0.0f, 0.0f,
  1.0f, 1.0f,

  // Bottom face top left
  0.0f, 1.0f,
  0.0f, 0.0f,
  1.0f, 0.0f,

  // Top face top right
  1.0f, 1.0f,
  1.0f, 0.0f,
  0.0f, 0.0f,

  // Left face bottom right
  0.0f, 0.0f,
  0.0f, 1.0f,
  1.0f, 0.0f,

  // Back face bottom right
  0.0f, 0.0f,
  0.0f, 1.0f,
  1.0f, 0.0f,

  // Right face bottom right
  0.0f, 0.0f,
  0.0f, 1.0f,
  1.0f, 0.0f,

  // Front face bottom right
  1.0f, 1.0f,
  0.0f, 0.0f,
  1.0f, 0.0f,

  // Bottom face bottom right
  1.0f, 1.0f,
  0.0f, 1.0f,
  1.0f, 0.0f,

  // Top face bottom left
  0.0f, 1.0f,
  1.0f, 1.0f,
  0.0f, 0.0f
};

//----------------------------------------------------------------------------------------
// Constructor
A5::A5(const std::string & luaSceneFile)
  : m_luaSceneFile(luaSceneFile),
    m_positionAttribLocation(0),
    m_normalAttribLocation(0),
    m_cubeTexCoordsAttribLocation(0),
    m_vao_meshData(0),
    m_vbo_vertexPositions(0),
    m_vbo_vertexNormals(0),
    m_mouse_x_pos(0.0f),
    m_mouse_y_pos(0.0f),
    x_angle(0.0f),
    y_angle(0.0f),
    power(0.0f),
    power_change(0.0f),
    m_num_textures(0),
    m_is_flying(false),
    m_game_in_progress(false),
    m_game_over(false),
    m_num_birds_left(10)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A5::~A5()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A5::init()
{
  // Initialize the sound resources
  m_themeSoundEngine = createIrrKlangDevice();
  m_themeSoundEngine->play2D("Assets/sounds/theme_song.wav", GL_TRUE);

  m_collisionSoundEngine = createIrrKlangDevice();

  m_birdSoundEngine = createIrrKlangDevice();

  // Set the background colour.
  glClearColor(0.35, 0.35, 0.35, 1.0);

  createShaderProgram();

  glGenVertexArrays(1, &m_vao_meshData);
  enableVertexShaderInputSlots();

  processLuaSceneFile(m_luaSceneFile);

  // Load and decode all .obj files at once here.  You may add additional .obj files to
  // this list in order to support rendering additional mesh types.  All vertex
  // positions, and normals will be extracted and stored within the MeshConsolidator
  // class.
  unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
      getAssetFilePath("cube.obj"),
      getAssetFilePath("sphere.obj")
  });


  // Acquire the BatchInfoMap from the MeshConsolidator.
  meshConsolidator->getBatchInfoMap(m_batchInfoMap);

  // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
  uploadVertexDataToVbos(*meshConsolidator);

  mapVboDataToVertexShaderInputLocations();

  initPerspectiveMatrix();

  initViewMatrix();

  initLightSources();

  getPhysicsNodes(*m_rootNode);

  getBirdNode(*m_rootNode);

  getAnimationNodes(*m_rootNode);

  // Load texture 1
  loadTexture(getAssetFilePath("textures/container.jpg").c_str(), TextureType::JPG);

  // Load texture 2
  loadTexture(getAssetFilePath("textures/grass.jpg").c_str(), TextureType::JPG);

  m_skybox = std::shared_ptr<SkyBox>(new SkyBox);

  vector<std::string> faces = {
      getAssetFilePath("textures/Sunny_01B_right.jpg"),
      getAssetFilePath("textures/Sunny_01B_left.jpg"),
      getAssetFilePath("textures/Sunny_01B_up.jpg"),
      getAssetFilePath("textures/Sunny_01B_down.jpg"),
      getAssetFilePath("textures/Sunny_01B_back.jpg"),
      getAssetFilePath("textures/Sunny_01B_front.jpg")
  };

  // Load cube map
  m_skybox->loadCubeMap(faces);

  initLightPerspectiveMatrix();

  initLightViewMatrix();

  initDepthMap();

  // Map the texture to active texture ids
  GLuint shadowMapLocation = m_shader.getUniformLocation("shadowMap");
  m_shader.enable();
  glUniform1i(shadowMapLocation, 0);
  m_shader.disable();

  GLuint ourTextureLocation = m_shader.getUniformLocation("ourTexture");
  m_shader.enable();
  glUniform1i(ourTextureLocation, 1);
  m_shader.disable();

  GLuint skyboxTextureLocation = m_shader.getUniformLocation("skyBox");
  m_shader.enable();
  glUniform1i(skyboxTextureLocation, 2);
  m_shader.disable();

  // Exiting the current scope calls delete automatically on meshConsolidator freeing
  // all vertex data resources.  This is fine since we already copied this data to
  // VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
  // this point.
}

//----------------------------------------------------------------------------------------
void A5::processLuaSceneFile(const std::string & filename) {
  // This version of the code treats the Lua file as an Asset,
  // so that you'd launch the program with just the filename
  // of a puppet in the Assets/ directory.
  // std::string assetFilePath = getAssetFilePath(filename.c_str());
  // m_rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

  // This version of the code treats the main program argument
  // as a straightforward pathname.
  m_rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
  if (!m_rootNode) {
    std::cerr << "Could not open " << filename << std::endl;
  }
}

//----------------------------------------------------------------------------------------
void A5::createShaderProgram()
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
  m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
  m_shader.link();

  m_depthMapShader.generateProgramObject();
  m_depthMapShader.attachVertexShader( getAssetFilePath("depthMap_VertexShader.vs").c_str() );
  m_depthMapShader.attachFragmentShader( getAssetFilePath("depthMap_FragmentShader.fs").c_str() );
  m_depthMapShader.link();
}

//----------------------------------------------------------------------------------------
void A5::enableVertexShaderInputSlots()
{
  //-- Enable input slots for m_vao_meshData:
  {
    glBindVertexArray(m_vao_meshData);

    // Enable the vertex shader attribute location for "position" when rendering.
    m_positionAttribLocation = m_shader.getAttribLocation("position");
    glEnableVertexAttribArray(m_positionAttribLocation);

    // Enable the vertex shader attribute location for "normal" when rendering.
    m_normalAttribLocation = m_shader.getAttribLocation("normal");
    glEnableVertexAttribArray(m_normalAttribLocation);

    // Enable the vertex shader attribute for "cubeTexcoords" when rendering
    m_cubeTexCoordsAttribLocation = m_shader.getAttribLocation("cubeTexcoords");
    glEnableVertexAttribArray(m_cubeTexCoordsAttribLocation);

    CHECK_GL_ERRORS;
  }

  // Restore defaults
  glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A5::uploadVertexDataToVbos (
    const MeshConsolidator & meshConsolidator
) {
  // Generate VBO to store all vertex position data
  {
    glGenBuffers(1, &m_vbo_vertexPositions);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
        meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }

  // Generate VBO to store all vertex normal data
  {
    glGenBuffers(1, &m_vbo_vertexNormals);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);

    glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
        meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }

  // Generate CBO to store all uv coordinates for a textured cube
  {
    glGenBuffers(1, &m_vbo_uvCube);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_uvCube);

    glBufferData(GL_ARRAY_BUFFER, sizeof(m_uvCube),
        m_uvCube, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
void A5::mapVboDataToVertexShaderInputLocations()
{
  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao_meshData);

  // Tell GL how to map data from the vertex buffer "m_vbo_vertexPositions" into the
  // "position" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexPositions);
  glVertexAttribPointer(m_positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Tell GL how to map data from the vertex buffer "m_vbo_vertexNormals" into the
  // "normal" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vertexNormals);
  glVertexAttribPointer(m_normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Tell GL how to map data from the vertex buffer "m_vbo_uvCube" into the
  // "cubeTexcoords" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_uvCube);
  glVertexAttribPointer(m_cubeTexCoordsAttribLocation, 3, GL_FLOAT, GL_FALSE, 2*sizeof(float), nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A5::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perspective = glm::perspective(degreesToRadians(65.0f), aspect, 0.1f, 1000.0f);
}

//----------------------------------------------------------------------------------------
void A5::initViewMatrix() {
  m_view = glm::lookAt(vec3(0.0f, 10.0f, 20.0f), vec3(0.0f, 10.0f, 0.0f),
      vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A5::initLightSources() {
  // World-space position
  m_light.position = vec3(0.0f, 50.0f, -0.1f);
  m_light.rgbIntensity = vec3(0.75f); // White light
}

//----------------------------------------------------------------------------------------
void A5::initLightPerspectiveMatrix()
{
  m_lightPerspective = glm::ortho(-25.0f, 25.0f, -75.0f, 25.0f, 0.1f, 55.0f);
}

//----------------------------------------------------------------------------------------
void A5::initLightViewMatrix() {
  m_lightView = glm::lookAt(m_light.position, vec3(0.0f, 0.0f, 0.0f),
      vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A5::getPhysicsNodes(SceneNode &node) {
  if (node.m_nodeType == NodeType::PhysicsNode) {
    PhysicsNode * physicsNode = static_cast<PhysicsNode *>(&node);
    m_physicsNodes.push_back(physicsNode);
  }

  for (SceneNode * child : node.children) {
    getPhysicsNodes(*child);
  }
}

//----------------------------------------------------------------------------------------
void A5::getAnimationNodes(SceneNode &node) {
  if (node.m_nodeType == NodeType::AnimationNode) {
    AnimationNode * animationNode = static_cast<AnimationNode *>(&node);
    m_animationNodes.push_back(animationNode);
  }

  for (SceneNode * child : node.children) {
    getAnimationNodes(*child);
  }
}

//----------------------------------------------------------------------------------------
void A5::getBirdNode(SceneNode &node) {
  if (node.m_nodeType == NodeType::PhysicsNode) {
    PhysicsNode * physicsNode = static_cast<PhysicsNode *>(&node);
    if (physicsNode->m_objectType == ObjectType::Bird) {
      m_birdNode = physicsNode;
      m_birdNodeOriginalTrans = mat4(physicsNode->trans);
      return;
    }
  }

  for (SceneNode * child : node.children) {
    getBirdNode(*child);
  }
}

//----------------------------------------------------------------------------------------
void A5::uploadCommonSceneUniforms() {
  m_shader.enable();
  {
    //-- Set Perpsective matrix uniform for the scene:
    GLint location = m_shader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
    CHECK_GL_ERRORS;

    //-- Set LightPerspective matrix uniform for the scene:
    location = m_shader.getUniformLocation("LightPerspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_lightPerspective));
    CHECK_GL_ERRORS;

    //-- Set LightSource uniform for the scene:
    location = m_shader.getUniformLocation("light.position");
    glUniform3fv(location, 1, value_ptr(m_light.position));
    location = m_shader.getUniformLocation("light.rgbIntensity");
    glUniform3fv(location, 1, value_ptr(m_light.rgbIntensity));
    CHECK_GL_ERRORS;

    //-- Set background light ambient intensity
    location = m_shader.getUniformLocation("ambientIntensity");
    vec3 ambientIntensity(0.05f);
    glUniform3fv(location, 1, value_ptr(ambientIntensity));
    CHECK_GL_ERRORS;
  }
  m_shader.disable();
}

//----------------------------------------------------------------------------------------
void A5::uploadDepthMapSceneUniforms() {
  m_depthMapShader.enable();
  {
    GLint location = m_depthMapShader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_lightPerspective));
    CHECK_GL_ERRORS;
  }
  m_depthMapShader.disable();
}

//----------------------------------------------------------------------------------------
void A5::initDepthMap() {
  glGenFramebuffers(1, &m_fbo_depthMap);

  glGenTextures(1, &m_depthMapTexture);
  glBindTexture(GL_TEXTURE_2D, m_depthMapTexture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, m_windowWidth, m_windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_depthMap);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMapTexture, 0);

  glReadBuffer(GL_NONE);
  glDrawBuffer(GL_NONE); // No color buffer is drawn to.

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A5::appLogic()
{
  double dt = 1/ImGui::GetIO().Framerate;

  if (m_game_in_progress || m_game_over) {
    updateParticleSystems(dt);

    updatePhysicsNodes(dt);

    updateAnimationNodes(dt);
  }

  if (m_game_in_progress) {
    checkIfGameOver();
  }
}

void A5::checkIfGameOver()
{
  if (m_num_birds_left == 0) {
    m_game_over = true;
    m_game_in_progress = false;
    m_themeSoundEngine->stopAllSounds();
    m_themeSoundEngine->play2D("Assets/sounds/game_lose.wav");
    return;
  }

  for (PhysicsNode * physicsNode : m_physicsNodes)
  {
    if (physicsNode->m_objectType == ObjectType::Pig && !physicsNode->m_destroyed) {
      return;
    }
  }

  for (AnimationNode * animationNode : m_animationNodes)
  {
    if (animationNode->m_objectType == ObjectType::Pig && !animationNode->m_destroyed) {
      return;
    }
  }

  m_game_over = true;
  m_game_in_progress = false;
  m_themeSoundEngine->stopAllSounds();
  m_themeSoundEngine->play2D("Assets/sounds/game_win.wav");
}

bool isDead(ParticleSystem *particleSystem) {
  return particleSystem->m_life < -5.0;
}

//----------------------------------------------------------------------------------------
void A5::updateParticleSystems(double dt)
{
  for (ParticleSystem * particleSystem : m_particleSystems) {
    particleSystem->update(dt);
  }

  m_particleSystems.remove_if(isDead);
}

//----------------------------------------------------------------------------------------
void A5::updatePhysicsNodes(double dt)
{
  bool collide;
  for (PhysicsNode * physicsNode : m_physicsNodes) {
    physicsNode->translate(vec3(
      physicsNode->m_velocity.x*dt,
      physicsNode->m_velocity.y*dt,
      physicsNode->m_velocity.z*dt
    ));

    collide = checkCollision(physicsNode);
    if (collide) {
      physicsNode->translate(vec3(
        -physicsNode->m_velocity.x*dt,
        -physicsNode->m_velocity.y*dt,
        -physicsNode->m_velocity.z*dt
      ));

      // Calculate collision response here
      const vec3 velocity(
        0.0f,
        0.0f,
        0.0f
      );
      physicsNode->set_velocity(velocity);
    }
    else if (physicsNode->m_gravity) {
      const vec3 velocity(
        physicsNode->m_velocity.x,
        physicsNode->m_velocity.y + (-50.0 * dt),
        physicsNode->m_velocity.z
      );
      physicsNode->set_velocity(velocity);
    }
  }
}

bool A5::checkCollision(PhysicsNode *physicsNode1)
{
  Primitive *p1 = physicsNode1->m_primitive;
  Primitive *p2;
  PhysicsNode *physicsNode2;
  bool collision = false;

  for (PhysicsNode * physicsNode2 : m_physicsNodes) {
    p2 = physicsNode2->m_primitive;

    if (!physicsNode1->m_destroyed && !physicsNode2->m_destroyed && p1 != p2) {

      if (p1->m_type == "cube" && p2->m_type == "cube") {
        bool collisionX = p1->m_pos.x + (p1->m_size.x/2) >= p2->m_pos.x - (p2->m_size.x/2) && p2->m_pos.x + (p2->m_size.x/2) >= p1->m_pos.x - (p1->m_size.x/2);
        bool collisionY = p1->m_pos.y + (p1->m_size.y/2) >= p2->m_pos.y - (p2->m_size.y/2) && p2->m_pos.y + (p2->m_size.y/2) >= p1->m_pos.y - (p1->m_size.y/2);
        bool collisionZ = p1->m_pos.z + (p1->m_size.z/2) >= p2->m_pos.z - (p2->m_size.z/2) && p2->m_pos.z + (p2->m_size.z/2) >= p1->m_pos.z - (p1->m_size.z/2);

        if (collisionX && collisionY && collisionZ) {
          if (physicsNode1->m_objectType == ObjectType::Bird && physicsNode2->m_objectType == ObjectType::Pig) {
            destroyPhysicsNode(physicsNode2);
          }
          collision = true;
          break;
        }
      }

      if (p1->m_type == "sphere" && p2->m_type == "cube" || p1->m_type == "cube" && p2->m_type == "sphere") {
        vec3 aabb_half_extents(p2->m_size.x/2, p2->m_size.y/2, p2->m_size.z/2);
        vec3 difference = p1->m_pos - p2->m_pos;
        vec3 clamped = clamp(difference, -aabb_half_extents, aabb_half_extents);
        vec3 closest = p2->m_pos + clamped;
        difference = closest - p1->m_pos;

        if (length(difference) <= p1->m_size.x) { // assume perfect sphere
          if (physicsNode1->m_objectType == ObjectType::Bird && physicsNode2->m_objectType == ObjectType::Pig) {
            destroyPhysicsNode(physicsNode2);
          }
          collision = true;
          break;
        }
      }

      if (p1->m_type == "sphere" && p2->m_type == "sphere") {
        vec3 difference = p1->m_pos - p2->m_pos;

        if (length(difference) <= p1->m_size.x + p2->m_size.x) {
          if (physicsNode1->m_objectType == ObjectType::Bird && physicsNode2->m_objectType == ObjectType::Pig) {
            destroyPhysicsNode(physicsNode2);
          }
          collision = true;
          break;
        }
      }
    }
  }

  for (AnimationNode * animationNode : m_animationNodes) {
    p2 = animationNode->m_primitive;

    if (!physicsNode1->m_destroyed && !animationNode->m_destroyed && p1 != p2) {

      if (p1->m_type == "cube" && p2->m_type == "cube") {
        bool collisionX = p1->m_pos.x + (p1->m_size.x/2) >= p2->m_pos.x - (p2->m_size.x/2) && p2->m_pos.x + (p2->m_size.x/2) >= p1->m_pos.x - (p1->m_size.x/2);
        bool collisionY = p1->m_pos.y + (p1->m_size.y/2) >= p2->m_pos.y - (p2->m_size.y/2) && p2->m_pos.y + (p2->m_size.y/2) >= p1->m_pos.y - (p1->m_size.y/2);
        bool collisionZ = p1->m_pos.z + (p1->m_size.z/2) >= p2->m_pos.z - (p2->m_size.z/2) && p2->m_pos.z + (p2->m_size.z/2) >= p1->m_pos.z - (p1->m_size.z/2);

        if (collisionX && collisionY && collisionZ) {
          if (physicsNode1->m_objectType == ObjectType::Bird && animationNode->m_objectType == ObjectType::Pig) {
            destroyAnimationNode(animationNode);
          }
          collision = true;
          break;
        }
      }

      if (p1->m_type == "sphere" && p2->m_type == "cube" || p1->m_type == "cube" && p2->m_type == "sphere") {
        vec3 aabb_half_extents(p2->m_size.x/2, p2->m_size.y/2, p2->m_size.z/2);
        vec3 difference = p1->m_pos - p2->m_pos;
        vec3 clamped = clamp(difference, -aabb_half_extents, aabb_half_extents);
        vec3 closest = p2->m_pos + clamped;
        difference = closest - p1->m_pos;

        if (length(difference) <= p1->m_size.x) { // assume perfect sphere
          if (physicsNode1->m_objectType == ObjectType::Bird && animationNode->m_objectType == ObjectType::Pig) {
            destroyAnimationNode(animationNode);
          }
          collision = true;
          break;
        }
      }

      if (p1->m_type == "sphere" && p2->m_type == "sphere") {
        vec3 difference = p1->m_pos - p2->m_pos;

        if (length(difference) <= p1->m_size.x + p2->m_size.x) {
          if (physicsNode1->m_objectType == ObjectType::Bird && animationNode->m_objectType == ObjectType::Pig) {
            destroyAnimationNode(animationNode);
          }
          collision = true;
          break;
        }
      }
    }
  }

  return collision;
}

void A5::destroyPhysicsNode(PhysicsNode *physicsNode)
{
  physicsNode->m_destroyed = true;

  vec4 color(physicsNode->material.kd, 1.0);
  ParticleSystem *particleSystem = new ParticleSystem(0.2, color);
  particleSystem->m_position = physicsNode->m_primitive->m_pos;
  particleSystem->m_velocity = vec3(0.0, 0.3, 0.0);

  m_particleSystems.push_back(particleSystem);

  m_collisionSoundEngine->play2D("Assets/sounds/collision1.wav");
}

void A5::destroyAnimationNode(AnimationNode *animationNode)
{
  animationNode->m_destroyed = true;

  vec4 color(animationNode->material.kd, 1.0);
  ParticleSystem *particleSystem = new ParticleSystem(0.2, color);
  particleSystem->m_position = animationNode->m_primitive->m_pos;
  particleSystem->m_velocity = vec3(0.0, 0.3, 0.0);

  m_particleSystems.push_back(particleSystem);

  m_collisionSoundEngine->play2D("Assets/sounds/collision1.wav");
}

//----------------------------------------------------------------------------------------
void A5::updateAnimationNodes(double dt)
{
  for (AnimationNode * animationNode : m_animationNodes) {
    animationNode->updateKeyframe(dt);
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A5::guiLogic()
{
  if( !show_gui ) {
    return;
  }

  static bool firstRun(true);
  if (firstRun) {
    ImGui::SetNextWindowPos(ImVec2(50, 50));
    firstRun = false;
  }

  if (m_game_in_progress) {
    static bool showDebugWindow(false);
    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.99f);

    ImGui::Begin("Angry Birds 3D", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);

      ImGui::SliderFloat("Power", &power, -0.0f, 45.0f);
      ImGui::Text("Number of birds left: %d", m_num_birds_left);

    ImGui::End();

    updateCamera();

    updateBirdOrientation();
  }
  else if (m_game_over) {
    static bool showDebugWindow(false);
    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.99f);

    ImGui::Begin("", &showDebugWindow, ImVec2(500,500), opacity, windowFlags);

      if (m_num_birds_left > 0) {
        ImGui::Text("You won!!");
      }
      else {
        ImGui::Text("Aw, you lost :(");
      }

    ImGui::End();
  }
  else {
    static bool showDebugWindow(false);
    ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
    float opacity(0.99f);

    ImGui::Begin("", &showDebugWindow, ImVec2(500,500), opacity, windowFlags);

      ImGui::Text("Welcome to Angry Birds 3D!");

      ImGui::Text(
        "Instructions to play: \n"
        "Use W A S D keys to change the pitch and angle of the bird. \n"
        "Hold down the SPACE key to build up power, release to shoot the bird. \n"
        "Press the R key to set the next bird and shoot again.\n"
      );

      ImGui::Text("Click here now to begin!");

      if( ImGui::Button( "Let's Go!" ) ) {
        m_game_in_progress = true;
        m_themeSoundEngine->stopAllSounds();
        m_themeSoundEngine->play2D("Assets/sounds/game_start.wav", false);
      }

    ImGui::End();
  }
}

//----------------------------------------------------------------------------------------
void A5::updateCamera()
{
  if ( m_keys[GLFW_KEY_LEFT_SHIFT] && m_keys[GLFW_KEY_UP] ) {
    m_view = rotate(mat4(), -0.1f, vec3(1.0f, 0.0f, 0.0f)) * m_view;
  }
  if ( m_keys[GLFW_KEY_LEFT_SHIFT] && m_keys[GLFW_KEY_DOWN] ) {
    m_view = rotate(mat4(), 0.1f, vec3(1.0f, 0.0f, 0.0f)) * m_view;
  }

  // Camera rotation and translation
  if ( m_keys[GLFW_KEY_DOWN] ) {
    vec3 amount(0.0f, 0.0f, -1.0f);
    m_view = translate(mat4(), amount) * m_view;
  }
  if ( m_keys[GLFW_KEY_UP] ) {
    vec3 amount(0.0f, 0.0f, 1.0f);
    m_view = translate(mat4(), amount) * m_view;
  }
  if ( m_keys[GLFW_KEY_RIGHT] ) {
    m_view = rotate(mat4(), 0.1f, vec3(0.0f, 1.0f, 0.0f)) * m_view;
  }
  if ( m_keys[GLFW_KEY_LEFT] ) {
    m_view = rotate(mat4(), -0.1f, vec3(0.0f, 1.0f, 0.0f)) * m_view;
  }
}

void A5::updateBirdOrientation()
{
  // Bird orientation
  if (!m_is_flying) {
    if ( m_keys[GLFW_KEY_D] ) {
      if (x_angle < 90.0f) {
        x_angle += 2.0f;
        m_birdNode->set_transform(m_birdNodeOriginalTrans);
        m_birdNode->rotate('x', y_angle);
        m_birdNode->rotate('y', -x_angle);
      }
    }
    if ( m_keys[GLFW_KEY_A] ) {
      if (x_angle > -90.0f) {
        x_angle -= 2.0f;
        m_birdNode->set_transform(m_birdNodeOriginalTrans);
        m_birdNode->rotate('x', y_angle);
        m_birdNode->rotate('y', -x_angle);
      }
    }
    if ( m_keys[GLFW_KEY_S] ) {
      if (y_angle > 0.0f) {
        y_angle -= 1.0f;
        m_birdNode->set_transform(m_birdNodeOriginalTrans);
        m_birdNode->rotate('x', y_angle);
        m_birdNode->rotate('y', -x_angle);
      }
    }
    if ( m_keys[GLFW_KEY_W] ) {
      if (y_angle < 60.0f) {
        y_angle += 1.0f;
        m_birdNode->set_transform(m_birdNodeOriginalTrans);
        m_birdNode->rotate('x', y_angle);
        m_birdNode->rotate('y', -x_angle);
      }
    }
    if ( m_keys[GLFW_KEY_SPACE] ) {
      if (power < 45.0f) {
        power += power_change;
        power_change += 0.01f;
      }
    }
  }
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A5::updateDepthMapShaderUniforms(
    const GeometryNode & node
) {

  m_depthMapShader.enable();
  {
    //-- Set ModelView matrix:
    GLint location = m_depthMapShader.getUniformLocation("ModelView");
    mat4 modelView = m_lightView * node.trans;
    glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(modelView) );
    CHECK_GL_ERRORS;
  }
  m_depthMapShader.disable();

}


//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A5::updateShaderUniforms(
    const GeometryNode & node
) {

  m_shader.enable();
  {
    //-- Set ModelView matrix:
    GLint location = m_shader.getUniformLocation("ModelView");
    mat4 modelView = m_view * node.trans;
    glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(modelView) );
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("LightModelView");
    mat4 lightModelView = m_lightView * node.trans;
    glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(lightModelView) );
    CHECK_GL_ERRORS;

    //-- Set NormMatrix:
    location = m_shader.getUniformLocation("NormalMatrix");
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
    glUniformMatrix3fv( location, 1, GL_FALSE, value_ptr(normalMatrix)) ;
    CHECK_GL_ERRORS;



    if (node.texture == 0) {
      // Disable textures
      location = m_shader.getUniformLocation("apply_texture");
      glUniform1i( location, 0 );

      // Disable skybox reflection
      location = m_shader.getUniformLocation("apply_skybox_reflection");
      glUniform1i( location, 0 );

      //-- Set Material values:
      location = m_shader.getUniformLocation("material.kd");
      vec3 kd = node.material.kd;
      glUniform3fv( location, 1, value_ptr(kd) );
      CHECK_GL_ERRORS;

      location = m_shader.getUniformLocation("material.ks");
      vec3 ks = node.material.ks;
      glUniform3fv( location, 1, value_ptr(ks) );
      CHECK_GL_ERRORS;

      location = m_shader.getUniformLocation("material.shininess");
      glUniform1f( location, node.material.shininess) ;
      CHECK_GL_ERRORS;
    }
    else if (node.texture == -1) {
      // Disable textures
      location = m_shader.getUniformLocation("apply_texture");
      glUniform1i( location, 0 );

      // Enable skybox reflection
      location = m_shader.getUniformLocation("apply_skybox_reflection");
      glUniform1i( location, 1 );

      glActiveTexture( GL_TEXTURE2 );
      glBindTexture( GL_TEXTURE_CUBE_MAP, m_skybox->m_cubeTextureID );
      CHECK_GL_ERRORS;
    }
    else if (node.texture <= m_num_textures) {
      // Enable textures
      location = m_shader.getUniformLocation("apply_texture");
      glUniform1i( location, 1 );

      // Disable skybox reflection
      location = m_shader.getUniformLocation("apply_skybox_reflection");
      glUniform1i( location, 0 );

      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, node.texture );
      CHECK_GL_ERRORS;
    }
  }
  m_shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A5::draw() {

  glEnable( GL_DEPTH_TEST );

  uploadDepthMapSceneUniforms();

  renderDepthMap(*m_rootNode);

  uploadCommonSceneUniforms();

  renderSceneGraph(*m_rootNode);

  renderParticles();

  renderSkyBox();

  glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
void A5::renderNode(const SceneNode &node, unsigned int type) {
  if (node.m_nodeType == NodeType::SceneNode) {
    // Mult matrix stack
    mat4 newTransform = matrixStack.empty() ? node.trans : matrixStack.top() * node.trans;
    matrixStack.push(newTransform);
  }
  else if (node.m_nodeType == NodeType::GeometryNode) {
    // Mult matrix stack
    mat4 newTransform = matrixStack.empty() ? node.trans : matrixStack.top() * node.trans;
    matrixStack.push(newTransform);

    // Cast the SceneNode as a GeometryNode, deep copy it and apply the
    // individual model transformation on the node
    const GeometryNode * geometryNode = static_cast<const GeometryNode *>(&node);
    GeometryNode transformedGeometryNode = GeometryNode(*geometryNode);
    transformedGeometryNode.set_transform(newTransform);

    vec3 col = transformedGeometryNode.material.kd;

    // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
    BatchInfo batchInfo = m_batchInfoMap[transformedGeometryNode.meshId];

    if (type == 1) {
      updateShaderUniforms( transformedGeometryNode );

      m_shader.enable();
      glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
      m_shader.disable();
    }
    else if (type == 2) {
      updateDepthMapShaderUniforms( transformedGeometryNode );

      m_depthMapShader.enable();
      glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
      CHECK_GL_ERRORS;
      m_depthMapShader.disable();
    }
  }
  else if (node.m_nodeType == NodeType::PhysicsNode) {
    const PhysicsNode * physicsNode = static_cast<const PhysicsNode *>(&node);

    if (physicsNode->m_destroyed == true) {
      return;
    }

    // Mult matrix stack
    mat4 newTransform = matrixStack.empty() ? node.trans : matrixStack.top() * node.trans;

    matrixStack.push(newTransform);
  }
  else if (node.m_nodeType == NodeType::AnimationNode) {
    const AnimationNode * animationNode = static_cast<const AnimationNode *>(&node);

    if (animationNode->m_destroyed == true) {
      return;
    }

    // Mult matrix stack
    mat4 newTransform = matrixStack.empty() ? animationNode->m_keyframe_trans*animationNode->trans : matrixStack.top()*animationNode->m_keyframe_trans*animationNode->trans;
    matrixStack.push(newTransform);
  }

  for (const SceneNode * child : node.children) {
    renderNode(*child, type);
  }

  matrixStack.pop();
}

//----------------------------------------------------------------------------------------
void A5::renderSceneGraph(const SceneNode & root) {

  // Bind the VAO once here, and reuse for all GeometryNode rendering below.
  glBindVertexArray(m_vao_meshData);
  glActiveTexture( GL_TEXTURE0 );
  glBindTexture( GL_TEXTURE_2D, m_depthMapTexture );

  renderNode(root, 1);

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A5::renderDepthMap(const SceneNode & root) {

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_depthMap);
  glClear(GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(m_vao_meshData);

  glCullFace(GL_FRONT);
  renderNode(root, 2);
  glCullFace(GL_BACK);

  glBindVertexArray(0);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  CHECK_GL_ERRORS;
}


//----------------------------------------------------------------------------------------
void A5::renderSkyBox() {
  glDepthFunc(GL_LEQUAL);
  m_skybox->m_shader.enable();

  {
    glBindVertexArray(m_skybox->m_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_skybox->m_cubeTextureID);

    GLint location = m_skybox->m_shader.getUniformLocation("View");
    glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(m_view) );
    CHECK_GL_ERRORS;

    location = m_skybox->m_shader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
    CHECK_GL_ERRORS;

    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
  }

  m_skybox->m_shader.disable();
  glDepthMask(GL_TRUE);
}

//----------------------------------------------------------------------------------------
void A5::renderParticles() {
  for (ParticleSystem * particleSystem : m_particleSystems) {
    particleSystem->m_shader.enable();
    glBindVertexArray(particleSystem->m_vao);

    for (Particle particle : particleSystem->m_particles) {
      if (particle.life > 0.0) {

        GLint location = particleSystem->m_shader.getUniformLocation("ModelView");
        mat4 model = translate(mat4(), particle.position);
        mat4 modelView = m_view * model;
        glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(modelView) );
        CHECK_GL_ERRORS;

        location = particleSystem->m_shader.getUniformLocation("Perspective");
        glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
        CHECK_GL_ERRORS;

        location = particleSystem->m_shader.getUniformLocation("color");
        glUniform4fv(location, 1, value_ptr(particleSystem->m_color));
        CHECK_GL_ERRORS;

        glDrawArrays(GL_TRIANGLES, 0, 6);
      }
    }

    glBindVertexArray(0);
    particleSystem->m_shader.disable();
  }
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


//----------------------------------------------------------------------------------------
void A5::loadTexture(const char* textureFilePath, TextureType type) {

  // Buffer to load texture into
  unsigned char *data;

  // Create a new texture object and bind it to the buffer
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // Set the texture wrapping/filtering options (on the currently bound texture object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // Load and generate the texture
  int width, height, nrChannels;
  if (type == TextureType::JPG) {
    data = stbi_load(textureFilePath, &width, &height, &nrChannels, 0);
  }
  else if (type == TextureType::PNG) {
    data = stbi_load(textureFilePath, &width, &height, &nrChannels, STBI_rgb_alpha);
  }

  if (data)
  {
      if (type == TextureType::JPG) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
      }
      else if (type == TextureType::PNG) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
      }
      glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
      cout << "Failed to load texture" << endl;
  }
  m_num_textures++;
  stbi_image_free(data);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A5::cleanup()
{
  m_themeSoundEngine->drop();
  m_birdSoundEngine->drop();
  m_collisionSoundEngine->drop();

  delete m_birdNode;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A5::cursorEnterWindowEvent (
    int entered
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool A5::mouseMoveEvent (
    double xPos,
    double yPos
) {
  bool eventHandled(false);

  m_mouse_x_pos = xPos;
  m_mouse_y_pos = yPos;

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A5::mouseButtonInputEvent (
    int button,
    int actions,
    int mods
) {
  bool eventHandled(false);

  if (actions == GLFW_PRESS && !ImGui::IsMouseHoveringAnyWindow()) {
    m_keys[button] = true;
  }

  if (actions == GLFW_RELEASE && !ImGui::IsMouseHoveringAnyWindow()) {
    m_keys[button] = false;
  }

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A5::mouseScrollEvent (
    double xOffSet,
    double yOffSet
) {
  bool eventHandled(false);

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A5::windowResizeEvent (
    int width,
    int height
) {
  bool eventHandled(false);
  initPerspectiveMatrix();
  initLightPerspectiveMatrix();
  initDepthMap();
  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A5::keyInputEvent (
    int key,
    int action,
    int mods
) {
  bool eventHandled(true);

  if ( action == GLFW_PRESS ) {
    if ( key == GLFW_KEY_M ) {
      show_gui = !show_gui;
      eventHandled = true;
    }
    else if (key == GLFW_KEY_R) {
      m_birdNode->set_transform(m_birdNodeOriginalTrans);
      m_birdNode->set_velocity(vec3(0.0f, 0.0f, 0.0f));
      x_angle = 0.0f;
      y_angle = 0.0f;
      power = 0.0f;
      power_change = 0.0f;
      m_is_flying = false;
      m_num_birds_left -= 1;
    }
    else {
      m_keys[key] = true;
    }
  }

  if ( action == GLFW_RELEASE ) {
    if ( key == GLFW_KEY_SPACE && !m_is_flying ) {
      double x_velocity, y_velocity, z_velocity;

      if (x_angle == 0.0f) {
        x_velocity = 0.0f;
        z_velocity = power;
      }
      else if (x_angle > 0.0f) {
        x_velocity = 1.0 * power;
        z_velocity = 1/(tan(degreesToRadians(x_angle))) * power;
      }
      else if (x_angle < 0.0f) {
        x_velocity = -1.0 * power;
        z_velocity = 1/(tan(degreesToRadians(-x_angle))) * power;
      }
      y_velocity = y_angle * power;

      m_birdNode->set_velocity(vec3(x_velocity, y_angle, -z_velocity));
      x_angle = 0.0f;
      y_angle = 0.0f;
      power = 0.0f;
      power_change = 0.0f;

      int sound = rand() % 5;
      m_collisionSoundEngine->play2D(flyingSounds[sound].c_str());
      m_is_flying = true;
    }
    m_keys[key] = false;
  }

  return eventHandled;
}

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

using namespace glm;

static bool show_gui = true;
stack<mat4> A3::matrixStack;

const size_t CIRCLE_PTS = 48;

//----------------------------------------------------------------------------------------
// Constructor
A3::A3(const std::string & luaSceneFile)
  : m_luaSceneFile(luaSceneFile),
    m_positionAttribLocation(0),
    m_normalAttribLocation(0),
    m_vao_meshData(0),
    m_vbo_vertexPositions(0),
    m_vbo_vertexNormals(0),
    m_current_mode(GLFW_KEY_C),
    m_mouse_x_pos(0.0f),
    m_mouse_y_pos(0.0f),
    m_num_textures(0)
{

}

//----------------------------------------------------------------------------------------
// Destructor
A3::~A3()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A3::init()
{
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

  // Load texture 1
  loadTexture("Assets/container.jpg");

  m_skybox = std::shared_ptr<SkyBox>(new SkyBox);

  vector<std::string> faces = {
      "Assets/right.jpg",
      "Assets/left.jpg",
      "Assets/top.jpg",
      "Assets/bottom.jpg",
      "Assets/back.jpg",
      "Assets/front.jpg"
  };

  // Load cube map
  m_skybox->loadCubeMap(faces);

  // Exiting the current scope calls delete automatically on meshConsolidator freeing
  // all vertex data resources.  This is fine since we already copied this data to
  // VBOs on the GPU.  We have no use for storing vertex data on the CPU side beyond
  // this point.
}

//----------------------------------------------------------------------------------------
void A3::processLuaSceneFile(const std::string & filename) {
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
void A3::createShaderProgram()
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
  m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
  m_shader.link();
}

//----------------------------------------------------------------------------------------
void A3::enableVertexShaderInputSlots()
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

    CHECK_GL_ERRORS;
  }

  // Restore defaults
  glBindVertexArray(0);
}

//----------------------------------------------------------------------------------------
void A3::uploadVertexDataToVbos (
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
}

//----------------------------------------------------------------------------------------
void A3::mapVboDataToVertexShaderInputLocations()
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

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perspective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}

//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
  m_view = glm::lookAt(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, 0.0f),
      vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
  // World-space position
  m_light.position = vec3(0.0f, 5.0f, 0.5f);
  m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
  m_shader.enable();
  {
    //-- Set Perpsective matrix uniform for the scene:
    GLint location = m_shader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perspective));
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
/*
 * Called once per frame, before guiLogic().
 */
void A3::appLogic()
{
  // Place per frame, application logic here ...

  uploadCommonSceneUniforms();
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A3::guiLogic()
{
  if( !show_gui ) {
    return;
  }

  static bool firstRun(true);
  if (firstRun) {
    ImGui::SetNextWindowPos(ImVec2(50, 50));
    firstRun = false;
  }

  static bool showDebugWindow(true);
  ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
  windowFlags |= ImGuiWindowFlags_MenuBar;
  float opacity(0.5f);

  ImGui::Begin("Assignment 5", &showDebugWindow, ImVec2(200,200), opacity, windowFlags);

    if (ImGui::BeginMenuBar())
    {
      if (ImGui::BeginMenu("Application"))
      {
        ImGui::MenuItem("Main menu bar");
        if (ImGui::MenuItem("Reset All")) {
        }
        if ( ImGui::MenuItem( "Quit Application" ) ) {
          glfwSetWindowShouldClose(m_window, GL_TRUE);
        }

        ImGui::EndMenu();
      }

      ImGui::EndMenuBar();
    }

    ImGui::RadioButton( "Translate Camera Mode", &m_current_mode, GLFW_KEY_C );
    ImGui::RadioButton( "Rotate Camera Mode", &m_current_mode, GLFW_KEY_R );

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
void A3::updateShaderUniforms(
    const GeometryNode & node
) {

  m_shader.enable();
  {
    //-- Set ModelView matrix:
    GLint location = m_shader.getUniformLocation("ModelView");
    mat4 modelView = m_view * node.trans;
    glUniformMatrix4fv( location, 1, GL_FALSE, value_ptr(modelView) );
    CHECK_GL_ERRORS;

    //-- Set NormMatrix:
    location = m_shader.getUniformLocation("NormalMatrix");
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
    glUniformMatrix3fv( location, 1, GL_FALSE, value_ptr(normalMatrix)) ;
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("apply_texture");

    if (node.texture == 0) {
      // Disable textures
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
    else if (node.texture <= m_num_textures) {
      glUniform1i( location, 1 );
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
void A3::draw() {

  glEnable( GL_DEPTH_TEST );

  renderSceneGraph(*m_rootNode);

  renderSkyBox();

  glDisable( GL_DEPTH_TEST );
}

//----------------------------------------------------------------------------------------
void A3::renderNode(const SceneNode &node) {
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
    // If this node is selected, assign the selected color
    if( SceneNode::selectedGeometryNodes[node.m_nodeId] ) {
      col = glm::vec3( 1.0, 1.0, 0.0 );
    }

    updateShaderUniforms( transformedGeometryNode );

    // Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
    BatchInfo batchInfo = m_batchInfoMap[transformedGeometryNode.meshId];

    // Render the mesh:
    m_shader.enable();
    glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
    m_shader.disable();
  }
  else if (node.m_nodeType == NodeType::JointNode) {
    // Cast the SceneNode as a JointNode
    const JointNode * jointNode = static_cast<const JointNode *>(&node);
    double angleX = JointNode::jointNodeX[jointNode->m_nodeId];
    double angleY = JointNode::jointNodeY[jointNode->m_nodeId];

    if (angleX > jointNode->m_joint_x.max) {
      angleX = jointNode->m_joint_x.max;
      JointNode::jointNodeX[jointNode->m_nodeId] = jointNode->m_joint_x.max;
    }
    else if (angleX < jointNode->m_joint_x.min) {
      angleX = jointNode->m_joint_x.min;
      JointNode::jointNodeX[jointNode->m_nodeId] = jointNode->m_joint_x.min;
    }

    if (angleY > jointNode->m_joint_y.max) {
      angleY = jointNode->m_joint_y.max;
      JointNode::jointNodeY[jointNode->m_nodeId] = jointNode->m_joint_y.max;
    }
    else if (angleY < jointNode->m_joint_y.min) {
      angleY = jointNode->m_joint_y.min;
      JointNode::jointNodeY[jointNode->m_nodeId] = jointNode->m_joint_y.min;
    }

    JointNode transformedJointNode = JointNode(*jointNode);
    transformedJointNode.rotate('x', angleX);
    transformedJointNode.rotate('y', angleY);

    matrixStack.push(matrixStack.top() * transformedJointNode.trans);
  }

  for (const SceneNode * child : node.children) {
    renderNode(*child);
  }

  matrixStack.pop();
}

//----------------------------------------------------------------------------------------
void A3::renderSceneGraph(const SceneNode & root) {

  // Bind the VAO once here, and reuse for all GeometryNode rendering below.
  glBindVertexArray(m_vao_meshData);

  renderNode(root);

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::renderSkyBox() {
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
void A3::loadTexture(const char* textureFilePath) {

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
  unsigned char *data = stbi_load(textureFilePath, &width, &height, &nrChannels, 0);
  if (data)
  {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
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
void A3::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A3::cursorEnterWindowEvent (
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
bool A3::mouseMoveEvent (
    double xPos,
    double yPos
) {
  bool eventHandled(false);

    // Camera mode event nahdling
  if (m_current_mode == GLFW_KEY_C) {
    if (!ImGui::IsMouseHoveringAnyWindow() && m_keys[GLFW_MOUSE_BUTTON_1]) {
      float xDiff = (xPos - m_mouse_x_pos)/m_windowHeight;
      float yDiff = (m_mouse_y_pos - yPos)/m_windowHeight;

      vec3 amount(xDiff*5, yDiff*5, 0.0f);
      mat4 transform = translate(mat4(), amount);

      m_view = transform * m_view;
    }
  }
  if (m_current_mode == GLFW_KEY_R) {
    if (!ImGui::IsMouseHoveringAnyWindow() && m_keys[GLFW_MOUSE_BUTTON_1]) {
      float xDiff = (xPos - m_mouse_x_pos)/m_windowHeight;
      mat4 transform = rotate(mat4(), xDiff*5, vec3(0.0f, 1.0f, 0.0f));

      m_view = transform * m_view;
    }
    if (!ImGui::IsMouseHoveringAnyWindow() && m_keys[GLFW_MOUSE_BUTTON_2]) {
      float yDiff = (m_mouse_y_pos - yPos)/m_windowHeight;
      mat4 transform = rotate(mat4(), yDiff*5, vec3(1.0f, 0.0f, 0.0f));

      m_view = transform * m_view;
    }
  }

  m_mouse_x_pos = xPos;
  m_mouse_y_pos = yPos;

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A3::mouseButtonInputEvent (
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
bool A3::mouseScrollEvent (
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
bool A3::windowResizeEvent (
    int width,
    int height
) {
  bool eventHandled(false);
  initPerspectiveMatrix();
  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A3::keyInputEvent (
    int key,
    int action,
    int mods
) {
  bool eventHandled(true);

  if( action == GLFW_PRESS ) {
    if( key == GLFW_KEY_M ) {
      show_gui = !show_gui;
      eventHandled = true;
    }
    else {
      m_current_mode = key;
    }
  }

  return eventHandled;
}

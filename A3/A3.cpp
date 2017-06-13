#include "A3.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "JointNode.hpp"

#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace glm;

static bool show_gui = true;
stack<mat4> A3::matrixStack;
vector< map < unsigned int, pair< double, double > > > A3::jointsAngleStack;

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
    m_vao_arcCircle(0),
    m_vbo_arcCircle(0),
    mouse_x_pos(0.0f),
    mouse_y_pos(0.0f),
    joints_angle_stack_index(-1)
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

  glGenVertexArrays(1, &m_vao_arcCircle);
  glGenVertexArrays(1, &m_vao_meshData);
  enableVertexShaderInputSlots();

  processLuaSceneFile(m_luaSceneFile);

  // Load and decode all .obj files at once here.  You may add additional .obj files to
  // this list in order to support rendering additional mesh types.  All vertex
  // positions, and normals will be extracted and stored within the MeshConsolidator
  // class.
  unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
      getAssetFilePath("cube.obj"),
      getAssetFilePath("sphere.obj"),
      getAssetFilePath("suzanne.obj")
  });


  // Acquire the BatchInfoMap from the MeshConsolidator.
  meshConsolidator->getBatchInfoMap(m_batchInfoMap);

  // Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
  uploadVertexDataToVbos(*meshConsolidator);

  mapVboDataToVertexShaderInputLocations();

  initModelMatrices();

  initPerspectiveMatrix();

  initViewMatrix();

  initLightSources();

  pushJointsAngleStack();

  do_picking = false;

  current_mode = GLFW_KEY_J;

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

  m_shader_arcCircle.generateProgramObject();
  m_shader_arcCircle.attachVertexShader( getAssetFilePath("arc_VertexShader.vs").c_str() );
  m_shader_arcCircle.attachFragmentShader( getAssetFilePath("arc_FragmentShader.fs").c_str() );
  m_shader_arcCircle.link();
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


  //-- Enable input slots for m_vao_arcCircle:
  {
    glBindVertexArray(m_vao_arcCircle);

    // Enable the vertex shader attribute location for "position" when rendering.
    m_arc_positionAttribLocation = m_shader_arcCircle.getAttribLocation("position");
    glEnableVertexAttribArray(m_arc_positionAttribLocation);

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

  // Generate VBO to store the trackball circle.
  {
    glGenBuffers( 1, &m_vbo_arcCircle );
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_arcCircle );

    float *pts = new float[ 2 * CIRCLE_PTS ];
    for( size_t idx = 0; idx < CIRCLE_PTS; ++idx ) {
      float ang = 2.0 * M_PI * float(idx) / CIRCLE_PTS;
      pts[2*idx] = cos( ang );
      pts[2*idx+1] = sin( ang );
    }

    glBufferData(GL_ARRAY_BUFFER, 2*CIRCLE_PTS*sizeof(float), pts, GL_STATIC_DRAW);

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

  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao_arcCircle);

  // Tell GL how to map data from the vertex buffer "m_vbo_arcCircle" into the
  // "position" vertex attribute location for any bound vertex shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_arcCircle);
  glVertexAttribPointer(m_arc_positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A3::initPerspectiveMatrix()
{
  float aspect = ((float)m_windowWidth) / m_windowHeight;
  m_perpsective = glm::perspective(degreesToRadians(60.0f), aspect, 0.1f, 100.0f);
}

//----------------------------------------------------------------------------------------
void A3::initModelMatrices() {
  m_model_rotation = mat4();
  m_model_translation = mat4();
}

//----------------------------------------------------------------------------------------
void A3::initViewMatrix() {
  m_view = glm::lookAt(vec3(0.0f, 0.0f, 5.0f), vec3(0.0f, 0.0f, -1.0f),
      vec3(0.0f, 1.0f, 0.0f));
}

//----------------------------------------------------------------------------------------
void A3::initLightSources() {
  // World-space position
  m_light.position = vec3(-2.0f, 5.0f, 0.5f);
  m_light.rgbIntensity = vec3(0.8f); // White light
}

//----------------------------------------------------------------------------------------
void traverseAndAddJointsToStack(
  const SceneNode & node,
  map < unsigned int, pair< double, double > > &jointsAngleMap
) {
  // Traverse hierarchical data structure and add all joints to the stack
  if (node.m_nodeType == NodeType::JointNode) {
    // Add angles to map if node is a JointNode
    jointsAngleMap[node.m_nodeId] = make_pair(
      JointNode::jointNodeX[node.m_nodeId],
      JointNode::jointNodeY[node.m_nodeId]
    );
  }

  for (const SceneNode * child : node.children) {
    traverseAndAddJointsToStack(*child, jointsAngleMap);
  }
}

//----------------------------------------------------------------------------------------
void A3::clearJointsAngleStack() {
  // Traverse hierarchical data structure and add all joints to the stack
  jointsAngleStack.erase(jointsAngleStack.begin() + 1, jointsAngleStack.end());
  moveJointsAngleStackIndex(-joints_angle_stack_index);
}

//----------------------------------------------------------------------------------------
void A3::pushJointsAngleStack() {
  // Traverse hierarchical data structure and add all joints to the stack
  map < unsigned int, pair< double, double > > jointsAngleMap;
  traverseAndAddJointsToStack(*m_rootNode, jointsAngleMap);

  if (joints_angle_stack_index < jointsAngleStack.size() - 1) {
    jointsAngleStack.erase(jointsAngleStack.begin() + joints_angle_stack_index + 1, jointsAngleStack.end());
  }

  jointsAngleStack.push_back(jointsAngleMap);
  joints_angle_stack_index++;
}

//----------------------------------------------------------------------------------------
void A3::moveJointsAngleStackIndex(int amount) {
  if (joints_angle_stack_index + amount >= 0 && joints_angle_stack_index + amount < jointsAngleStack.size()) {
    joints_angle_stack_index += amount;
    map < unsigned int, pair< double, double > > jointsAngleMap = jointsAngleStack[joints_angle_stack_index];
    map < unsigned int, pair< double, double > >::iterator it;

    for (it=jointsAngleMap.begin(); it!=jointsAngleMap.end(); it++) {
      unsigned int nodeId = it->first;
      double angleX = it->second.first;
      double angleY = it->second.second;

      JointNode::jointNodeX[nodeId] = angleX;
      JointNode::jointNodeY[nodeId] = angleY;
    }
  }
}

//----------------------------------------------------------------------------------------
void A3::uploadCommonSceneUniforms() {
  m_shader.enable();
  {
    //-- Set Perpsective matrix uniform for the scene:
    GLint location = m_shader.getUniformLocation("Perspective");
    glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(m_perpsective));
    CHECK_GL_ERRORS;

    location = m_shader.getUniformLocation("picking");
    glUniform1i( location, do_picking ? 1 : 0 );

    //-- Set LightSource uniform for the scene:
    if (!do_picking) {
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
  float opacity(0.5f);

  ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
      windowFlags);


    // Add more gui elements here here ...
    ImGui::RadioButton( "Joints", &current_mode, GLFW_KEY_J );
    ImGui::RadioButton( "Position/Orientation", &current_mode, GLFW_KEY_P );

    // Create Button, and check if it was clicked:
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

//----------------------------------------------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
    const ShaderProgram & shader,
    const GeometryNode & node,
    const glm::mat4 & viewMatrix,
    unsigned int node_id,
    bool do_picking,
    const glm::vec3& col
) {

  shader.enable();

  //-- Set ModelView matrix:
  GLint location = shader.getUniformLocation("ModelView");
  mat4 modelView = viewMatrix * node.trans;
  glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(modelView));
  CHECK_GL_ERRORS;

  if (do_picking) {
    float r = float(node_id&0xff) / 255.0f;
    float g = float((node_id>>8)&0xff) / 255.0f;
    float b = float((node_id>>16)&0xff) / 255.0f;

    location = shader.getUniformLocation("material.kd");
    glUniform3f( location, r, g, b );
    CHECK_GL_ERRORS;
  }
  else {
    //-- Set NormMatrix:
    location = shader.getUniformLocation("NormalMatrix");
    mat3 normalMatrix = glm::transpose(glm::inverse(mat3(modelView)));
    glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
    CHECK_GL_ERRORS;


    //-- Set Material values:
    location = shader.getUniformLocation("material.kd");
    vec3 kd = col;
    glUniform3fv(location, 1, value_ptr(kd));
    CHECK_GL_ERRORS;
    location = shader.getUniformLocation("material.ks");
    vec3 ks = node.material.ks;
    glUniform3fv(location, 1, value_ptr(ks));
    CHECK_GL_ERRORS;
    location = shader.getUniformLocation("material.shininess");
    glUniform1f(location, node.material.shininess);
    CHECK_GL_ERRORS;
  }

  shader.disable();

}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A3::draw() {

  glEnable( GL_DEPTH_TEST );
  glEnable( GL_CULL_FACE );
  glCullFace( GL_BACK );

  renderSceneGraph(*m_rootNode);

  glDisable( GL_DEPTH_TEST );
  glDisable( GL_CULL_FACE );

  renderArcCircle();
}

void A3::renderNode(const SceneNode &node) {
  if (node.m_nodeType == NodeType::SceneNode) {
    // Mult matrix stack
    mat4 newTransform = matrixStack.empty() ? node.trans : matrixStack.top();
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

    updateShaderUniforms(
      m_shader,
      transformedGeometryNode,
      m_view,
      node.m_nodeId,
      do_picking,
      col
    );

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

  matrixStack.push(m_model_translation * m_model_rotation);
  renderNode(root);
  matrixStack.pop();

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
// Draw the trackball circle.
void A3::renderArcCircle() {
  glBindVertexArray(m_vao_arcCircle);

  m_shader_arcCircle.enable();
    GLint m_location = m_shader_arcCircle.getUniformLocation( "M" );
    float aspect = float(m_framebufferWidth)/float(m_framebufferHeight);
    glm::mat4 M;
    if( aspect > 1.0 ) {
      M = glm::scale( glm::mat4(), glm::vec3( 0.5/aspect, 0.5, 1.0 ) );
    } else {
      M = glm::scale( glm::mat4(), glm::vec3( 0.5, 0.5*aspect, 1.0 ) );
    }
    glUniformMatrix4fv( m_location, 1, GL_FALSE, value_ptr( M ) );
    glDrawArrays( GL_LINE_LOOP, 0, CIRCLE_PTS );
  m_shader_arcCircle.disable();

  glBindVertexArray(0);
  CHECK_GL_ERRORS;
}

vec3 vCalcRotVec(float fNewX, float fNewY,
                 float fOldX, float fOldY,
                 float fDiameter) {
   long  nXOrigin, nYOrigin;
   float fNewVecX, fNewVecY, fNewVecZ,        /* Vector corresponding to new mouse location */
         fOldVecX, fOldVecY, fOldVecZ,        /* Vector corresponding to old mouse location */
         fLength;

   fNewVecX    = fNewX * 2.0 / fDiameter;
   fNewVecY    = fNewY * 2.0 / fDiameter;
   fNewVecZ    = (1.0 - fNewVecX * fNewVecX - fNewVecY * fNewVecY);

   if (fNewVecZ < 0.0) {
      fLength = sqrt(1.0 - fNewVecZ);
      fNewVecZ  = 0.0;
      fNewVecX /= fLength;
      fNewVecY /= fLength;
   } else {
      fNewVecZ = sqrt(fNewVecZ);
   }

   fOldVecX    = fOldX * 2.0 / fDiameter;
   fOldVecY    = fOldY * 2.0 / fDiameter;
   fOldVecZ    = (1.0 - fOldVecX * fOldVecX - fOldVecY * fOldVecY);

   if (fOldVecZ < 0.0) {
      fLength = sqrt(1.0 - fOldVecZ);
      fOldVecZ  = 0.0;
      fOldVecX /= fLength;
      fOldVecY /= fLength;
   } else {
      fOldVecZ = sqrt(fOldVecZ);
   }

   vec3 fVec;
   fVec[0] = fOldVecY * fNewVecZ - fNewVecY * fOldVecZ;
   fVec[1] = fOldVecZ * fNewVecX - fNewVecZ * fOldVecX;
   fVec[2] = fOldVecX * fNewVecY - fNewVecX * fOldVecY;

   return fVec;
}

mat4 vAxisRotMatrix(float fVecX, float fVecY, float fVecZ) {
    float fRadians, fInvLength, fNewVecX, fNewVecY, fNewVecZ;
    mat4 mNewMat;

    fRadians = sqrt(fVecX * fVecX + fVecY * fVecY + fVecZ * fVecZ);

    if (fRadians > -0.000001 && fRadians < 0.000001) {
        return mNewMat;
    }

    fInvLength = 1 / fRadians;
    fNewVecX   = fVecX * fInvLength;
    fNewVecY   = fVecY * fInvLength;
    fNewVecZ   = fVecZ * fInvLength;

    double dSinAlpha = sin(fRadians);
    double dCosAlpha = cos(fRadians);
    double dT = 1 - dCosAlpha;

    mNewMat[0][0] = dCosAlpha + fNewVecX*fNewVecX*dT;
    mNewMat[0][1] = fNewVecX*fNewVecY*dT + fNewVecZ*dSinAlpha;
    mNewMat[0][2] = fNewVecX*fNewVecZ*dT - fNewVecY*dSinAlpha;
    mNewMat[0][3] = 0;

    mNewMat[1][0] = fNewVecX*fNewVecY*dT - dSinAlpha*fNewVecZ;
    mNewMat[1][1] = dCosAlpha + fNewVecY*fNewVecY*dT;
    mNewMat[1][2] = fNewVecY*fNewVecZ*dT + dSinAlpha*fNewVecX;
    mNewMat[1][3] = 0;

    mNewMat[2][0] = fNewVecZ*fNewVecX*dT + dSinAlpha*fNewVecY;
    mNewMat[2][1] = fNewVecZ*fNewVecY*dT - dSinAlpha*fNewVecX;
    mNewMat[2][2] = dCosAlpha + fNewVecZ*fNewVecZ*dT;
    mNewMat[2][3] = 0;

    mNewMat[3][0] = 0;
    mNewMat[3][1] = 0;
    mNewMat[3][2] = 0;
    mNewMat[3][3] = 1;

    return mNewMat;
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
  // Position/Orientation
  if (current_mode == GLFW_KEY_P) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float xDiff = (xPos - mouse_x_pos)/m_windowHeight;
      float yDiff = (mouse_y_pos - yPos)/m_windowHeight;

      vec3 amount(xDiff*5, yDiff*5, 0.0f);
      mat4 transform = translate(mat4(), amount);

      m_model_translation *= transform;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      // Trackball stuff
      float diamater = m_framebufferHeight > m_framebufferWidth ? m_framebufferWidth/2 : m_framebufferHeight/2;
      vec3 rotation = vCalcRotVec(
        xPos - m_framebufferWidth/2,
        yPos - m_framebufferHeight/2,
        mouse_x_pos - m_framebufferWidth/2,
        mouse_y_pos - m_framebufferHeight/2,
        diamater
      );
      mat4 transform = vAxisRotMatrix(rotation[0], rotation[1], rotation[2]);

      m_model_rotation *= transform;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float yDiff = (yPos - mouse_y_pos)/m_windowHeight;

      vec3 amount(0.0f, 0.0f, yDiff*5);
      mat4 transform = translate(mat4(), amount);

      m_model_translation *= transform;
    }
  }

  // Joints rotation
  if (current_mode == GLFW_KEY_J) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float diff = (xPos - mouse_x_pos)*100/m_windowWidth;

      vector<bool>::iterator it;
      for (it=SceneNode::selectedGeometryNodes.begin(); it!=SceneNode::selectedGeometryNodes.end(); it++) {
        int index = it - SceneNode::selectedGeometryNodes.begin();
        int jointNodeIndex = SceneNode::geometryNodesToJoints[index];
        if (*it && (jointNodeIndex != -1)) {
          JointNode::jointNodeY[jointNodeIndex] += diff;
        }
      }
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float diff = (yPos - mouse_y_pos)*100/m_windowHeight;

      vector<bool>::iterator it;
      for (it=SceneNode::selectedGeometryNodes.begin(); it!=SceneNode::selectedGeometryNodes.end(); it++) {
        int index = it - SceneNode::selectedGeometryNodes.begin();
        int jointNodeIndex = SceneNode::geometryNodesToJoints[index];
        if (*it && (jointNodeIndex != -1)) {
          JointNode::jointNodeX[jointNodeIndex] += diff;
        }
      }
    }


  }

  mouse_x_pos = xPos;
  mouse_y_pos = yPos;

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
    if (button == GLFW_MOUSE_BUTTON_1) {
      keys[GLFW_MOUSE_BUTTON_1] = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_2) {
      keys[GLFW_MOUSE_BUTTON_2] = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_3) {
      keys[GLFW_MOUSE_BUTTON_3] = true;
    }

    // Joints mode
    if (current_mode == GLFW_KEY_J) {
      if (button == GLFW_MOUSE_BUTTON_1) {

        double xpos, ypos;
        glfwGetCursorPos( m_window, &xpos, &ypos );
        do_picking = true;

        uploadCommonSceneUniforms();
        glClearColor(1.0, 1.0, 1.0, 1.0 );
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
        glClearColor(0.35, 0.35, 0.35, 1.0);

        draw();

        CHECK_GL_ERRORS;

        // Ugly -- FB coordinates might be different than Window coordinates
        // (e.g., on a retina display).  Must compensate.
        xpos *= double(m_framebufferWidth) / double(m_windowWidth);
        // WTF, don't know why I have to measure y relative to the bottom of
        // the window in this case.
        ypos = m_windowHeight - ypos;
        ypos *= double(m_framebufferHeight) / double(m_windowHeight);

        GLubyte buffer[ 4 ] = { 0, 0, 0, 0 };
        // A bit ugly -- don't want to swap the just-drawn false colours
        // to the screen, so read from the back buffer.
        glReadBuffer( GL_BACK );
        // Actually read the pixel at the mouse location.
        glReadPixels( int(xpos), int(ypos), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer );
        CHECK_GL_ERRORS;

        // Reassemble the object ID.
        unsigned int what = buffer[0] + (buffer[1] << 8) + (buffer[2] << 16);

        // Only mark the geometry node as selected if it is connected to a joint
        if (what < SceneNode::totalSceneNodes() && SceneNode::geometryNodesToJoints[what] != -1) {
          SceneNode::selectedGeometryNodes[what] = !SceneNode::selectedGeometryNodes[what];
        }

        do_picking = false;

        CHECK_GL_ERRORS;
      }
    }

  }

  if (actions == GLFW_RELEASE && !ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      keys[GLFW_MOUSE_BUTTON_1] = false;
    }
    if (button == GLFW_MOUSE_BUTTON_2) {
      keys[GLFW_MOUSE_BUTTON_2] = false;
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
      keys[GLFW_MOUSE_BUTTON_3] = false;
    }

    // Joints mode
    if (current_mode == GLFW_KEY_J) {
      if (button == GLFW_MOUSE_BUTTON_2 || button == GLFW_MOUSE_BUTTON_3) {
        pushJointsAngleStack();
      }
    }
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

  // Fill in with event handling code...

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
    else if ( key == GLFW_KEY_O ) {
      initModelMatrices();
    }
    else if ( key == GLFW_KEY_N ) {
      clearJointsAngleStack();
    }
    else if ( key == GLFW_KEY_Q ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }
    else if (key != GLFW_KEY_R && key != GLFW_KEY_U) {
      current_mode = key;
    }

    if (current_mode == GLFW_KEY_J) {
      if (key == GLFW_KEY_U) {
        moveJointsAngleStackIndex(-1);
      }
      if (key == GLFW_KEY_R) {
        moveJointsAngleStackIndex(1);
      }
    }
  }
  // Fill in with event handling code...

  return eventHandled;
}

#include "A2.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
using namespace std;

#include <imgui/imgui.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
using namespace glm;

//----------------------------------------------------------------------------------------
// Constructor
VertexData::VertexData()
  : numVertices(0),
    index(0)
{
  positions.resize(kMaxVertices);
  colours.resize(kMaxVertices);
}


//----------------------------------------------------------------------------------------
// Constructor
A2::A2()
  : m_currentLineColour(vec3(0.0f))
{
  mouse_x_pos = 0.0f;
  mouse_y_pos = 0.0f;
  zoom = 0.0f;
  is_left_mouse_clicked = false;
  is_middle_mouse_clicked = false;
  is_right_mouse_clicked = false;
}

//----------------------------------------------------------------------------------------
// Destructor
A2::~A2()
{

}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A2::init()
{
  // Set the background colour.
  glClearColor(0.3, 0.5, 0.7, 1.0);

  createShaderProgram();

  glGenVertexArrays(1, &m_vao);

  enableVertexAttribIndices();

  generateVertexBuffers();

  mapVboDataToVertexAttributeLocation();

  initializeCoordinateFrames();

  initializeModelCoordinates();

  initializeTransformationMatrices();
}

//----------------------------------------------------------------------------------------
void A2::createShaderProgram()
{
  m_shader.generateProgramObject();
  m_shader.attachVertexShader( getAssetFilePath("VertexShader.vs").c_str() );
  m_shader.attachFragmentShader( getAssetFilePath("FragmentShader.fs").c_str() );
  m_shader.link();
}

//----------------------------------------------------------------------------------------
void A2::enableVertexAttribIndices()
{
  glBindVertexArray(m_vao);

  // Enable the attribute index location for "position" when rendering.
  GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray(positionAttribLocation);

  // Enable the attribute index location for "colour" when rendering.
  GLint colourAttribLocation = m_shader.getAttribLocation( "colour" );
  glEnableVertexAttribArray(colourAttribLocation);

  // Restore defaults
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::generateVertexBuffers()
{
  // Generate a vertex buffer to store line vertex positions
  {
    glGenBuffers(1, &m_vbo_positions);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);

    // Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * kMaxVertices, nullptr,
        GL_DYNAMIC_DRAW);


    // Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }

  // Generate a vertex buffer to store line colors
  {
    glGenBuffers(1, &m_vbo_colours);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);

    // Set to GL_DYNAMIC_DRAW because the data store will be modified frequently.
    glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * kMaxVertices, nullptr,
        GL_DYNAMIC_DRAW);


    // Unbind the target GL_ARRAY_BUFFER, now that we are finished using it.
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
void A2::mapVboDataToVertexAttributeLocation()
{
  // Bind VAO in order to record the data mapping.
  glBindVertexArray(m_vao);

  // Tell GL how to map data from the vertex buffer "m_vbo_positions" into the
  // "position" vertex attribute index for any bound shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
  GLint positionAttribLocation = m_shader.getAttribLocation( "position" );
  glVertexAttribPointer(positionAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  // Tell GL how to map data from the vertex buffer "m_vbo_colours" into the
  // "colour" vertex attribute index for any bound shader program.
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
  GLint colorAttribLocation = m_shader.getAttribLocation( "colour" );
  glVertexAttribPointer(colorAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  //-- Unbind target, and restore default values:
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
void A2::initializeCoordinateFrames()
{
  f_model.resize(0);
  f_model.push_back(vec4(1.0f, 0.0f, 0.0f, 0.0f));
  f_model.push_back(vec4(0.0f, 1.0f, 0.0f, 0.0f));
  f_model.push_back(vec4(0.0f, 0.0f, 1.0f, 0.0f));
  f_model.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));

  f_world.resize(0);
  f_world.push_back(vec4(1.0f, 0.0f, 0.0f, 0.0f));
  f_world.push_back(vec4(0.0f, 1.0f, 0.0f, 0.0f));
  f_world.push_back(vec4(0.0f, 0.0f, 1.0f, 0.0f));
  f_world.push_back(vec4(0.0f, 0.0f, 0.0f, 1.0f));

  f_view.resize(0);
  f_view.push_back(vec4(1.0f, 0.0f, 0.0f, 0.0f));
  f_view.push_back(vec4(0.0f, 1.0f, 0.0f, 0.0f));
  f_view.push_back(vec4(0.0f, 0.0f, 1.0f, 0.0f));
  f_view.push_back(vec4(1.0f, 1.0f, 1.0f, 1.0f));

  // f_view.push_back(vec4(pow(2.0f, 0.5f)/2, pow(2.0f, 0.5f)/2, 0.0f, 0.0f));
  // f_view.push_back(vec4(0.0f, 0.0f, 1.0f, 0.0f));
  // f_view.push_back(vec4(pow(2.0f, 0.5f)/2, -pow(2.0f, 0.5f)/2, 0.0f, 0.0f));
  // f_view.push_back(vec4(1.0f, 0.0f, 3.0f, 1.0f));
}

//----------------------------------------------------------------------------------------
void A2::initializeModelCoordinates()
{
  // Initialize local coordinates for cube
  model_coordinates.push_back( vec4(-0.5f, -0.5f, -0.5f, 1.0f) ); // left bottom front
  model_coordinates.push_back( vec4(-0.5f, 0.5f, -0.5f, 1.0f) ); // left top front
  model_coordinates.push_back( vec4(0.5f, -0.5f, -0.5f, 1.0f) ); // right bottom front
  model_coordinates.push_back( vec4(0.5f, 0.5f, -0.5f, 1.0f) ); // right top front

  model_coordinates.push_back( vec4(-0.5f, -0.5f, 0.5f, 1.0f) ); // left bottom back
  model_coordinates.push_back( vec4(-0.5f, 0.5f, 0.5f, 1.0f) ); // left top back
  model_coordinates.push_back( vec4(0.5f, -0.5f, 0.5f, 1.0f) ); // right bottom back
  model_coordinates.push_back( vec4(0.5f, 0.5f, 0.5f, 1.0f) ); // right top back
}

//----------------------------------------------------------------------------------------
void A2::initializeTransformationMatrices()
{
  t_model = translate( mat4(1.0f), vec3(0.0f, 0.0f, 0.0f) );

  t_view = glm::lookAt(
    glm::vec3( 0.0f, 0.0f, 10.0f ),
    glm::vec3( 0.0f, 0.0f, 0.0f ),
    glm::vec3( 0.0f, 1.0f, 0.0f ) );

  t_proj =  glm::perspective(
    glm::radians( 45.0f ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f, 1000.0f );

}

//----------------------------------------------------------------------------------------
void A2::applyModelTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  world_coordinates.resize(0);

  for (it=model_coordinates.begin(); it!=model_coordinates.end(); it++) {
    world_coordinates.push_back(t_model * (*it));
  }
}

//----------------------------------------------------------------------------------------
void A2::applyViewingTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  view_coordinates.resize(0);

  for (it=world_coordinates.begin(); it!=world_coordinates.end(); it++) {
    view_coordinates.push_back(t_view * (*it));
  }
}

//----------------------------------------------------------------------------------------
void A2::applyProjectionTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  float width_ratio = 0.9f;
  float height_ratio = 0.9f;
  normalized_device_coordinates.resize(0);

  for (it=view_coordinates.begin(); it!=view_coordinates.end(); it++) {
    // Remove z coordinate and normalize
    // vec2 device_coordinate = vec2(it->x/it->z, it->y/it->z);
    // cout << device_coordinate << endl;
    // vec2 normalized_device_coordinate = normalize(vec2(
    //   (width_ratio * device_coordinate.x) + 0.05*m_windowWidth,
    //   (height_ratio * device_coordinate.y) + 0.05*m_windowHeight
    // ));
    // normalized_device_coordinates.push_back(normalized_device_coordinate);
    normalized_device_coordinates.push_back(vec2(it->x, it->y));
  }

}


//---------------------------------------------------------------------------------------
void A2::initLineData()
{
  m_vertexData.numVertices = 0;
  m_vertexData.index = 0;
}

//---------------------------------------------------------------------------------------
void A2::setLineColour (
    const glm::vec3 & colour
) {
  m_currentLineColour = colour;
}

//---------------------------------------------------------------------------------------
void A2::drawLine(
    const glm::vec2 & v0,   // Line Start (NDC coordinate)
    const glm::vec2 & v1    // Line End (NDC coordinate)
) {

  m_vertexData.positions[m_vertexData.index] = v0;
  m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
  ++m_vertexData.index;
  m_vertexData.positions[m_vertexData.index] = v1;
  m_vertexData.colours[m_vertexData.index] = m_currentLineColour;
  ++m_vertexData.index;

  m_vertexData.numVertices += 2;
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A2::appLogic()
{
  // Place per frame, application logic here ...
  applyModelTransformation();
  applyViewingTransformation();
  applyProjectionTransformation();

  // Call at the beginning of frame, before drawing lines:
  initLineData();

  // Draw outer square:
  // setLineColour(vec3(1.0f, 0.7f, 0.8f));
  // drawLine(vec2(-0.5f, -0.5f), vec2(0.5f, -0.5f));
  // drawLine(vec2(0.5f, -0.5f), vec2(0.5f, 0.5f));
  // drawLine(vec2(0.5f, 0.5f), vec2(-0.5f, 0.5f));
  // drawLine(vec2(-0.5f, 0.5f), vec2(-0.5f, -0.5f));


  // // Draw inner square:
  // setLineColour(vec3(0.2f, 1.0f, 1.0f));
  // drawLine(vec2(-0.25f, -0.25f), vec2(0.25f, -0.25f));
  // drawLine(vec2(0.25f, -0.25f), vec2(0.25f, 0.25f));
  // drawLine(vec2(0.25f, 0.25f), vec2(-0.25f, 0.25f));
  // drawLine(vec2(-0.25f, 0.25f), vec2(-0.25f, -0.25f));

  setLineColour(vec3(1.0f, 0.7f, 0.8f));
  drawLine(normalized_device_coordinates[0], normalized_device_coordinates[1]);
  drawLine(normalized_device_coordinates[2], normalized_device_coordinates[3]);
  drawLine(normalized_device_coordinates[0], normalized_device_coordinates[2]);
  drawLine(normalized_device_coordinates[1], normalized_device_coordinates[3]);

  drawLine(normalized_device_coordinates[4], normalized_device_coordinates[5]);
  drawLine(normalized_device_coordinates[6], normalized_device_coordinates[7]);
  drawLine(normalized_device_coordinates[4], normalized_device_coordinates[6]);
  drawLine(normalized_device_coordinates[5], normalized_device_coordinates[7]);

  drawLine(normalized_device_coordinates[0], normalized_device_coordinates[4]);
  drawLine(normalized_device_coordinates[2], normalized_device_coordinates[6]);
  drawLine(normalized_device_coordinates[1], normalized_device_coordinates[5]);
  drawLine(normalized_device_coordinates[3], normalized_device_coordinates[7]);
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A2::guiLogic()
{
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


    // Create Button, and check if it was clicked:
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();
}

//----------------------------------------------------------------------------------------
void A2::uploadVertexDataToVbos() {

  //-- Copy vertex position data into VBO, m_vbo_positions:
  {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_positions);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec2) * m_vertexData.numVertices,
        m_vertexData.positions.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }

  //-- Copy vertex colour data into VBO, m_vbo_colours:
  {
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_colours);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vec3) * m_vertexData.numVertices,
        m_vertexData.colours.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    CHECK_GL_ERRORS;
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A2::draw()
{
  uploadVertexDataToVbos();

  glBindVertexArray(m_vao);

  m_shader.enable();
    glDrawArrays(GL_LINES, 0, m_vertexData.numVertices);
  m_shader.disable();

  // Restore defaults
  glBindVertexArray(0);

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A2::cleanup()
{

}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A2::cursorEnterWindowEvent (
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
bool A2::mouseMoveEvent (
    double xPos,
    double yPos
) {
  bool eventHandled(false);

  if (!ImGui::IsMouseHoveringAnyWindow() && is_right_mouse_clicked) {
    float y_rotation = (xPos - mouse_x_pos) / (100*2*M_PI);
    mat4 transform = rotate(mat4(), y_rotation, vec3(0.0f, 1.0f, 0.0f));
    t_view *= transform;
  }

  if (!ImGui::IsMouseHoveringAnyWindow() && is_left_mouse_clicked) {
    float x_rotation = (yPos - mouse_y_pos) / (100*2*M_PI);
    mat4 transform = rotate(mat4(), x_rotation, vec3(1.0f, 0.0f, 0.0f));
    t_view *= transform;
  }

  mouse_y_pos = yPos;
  mouse_x_pos = xPos;

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A2::mouseButtonInputEvent (
    int button,
    int actions,
    int mods
) {
  bool eventHandled(false);

  if (actions == GLFW_PRESS && !ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      is_left_mouse_clicked = true;
    }
    if (button == GLFW_MOUSE_BUTTON_2) {
      is_right_mouse_clicked = true;
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
      is_left_mouse_clicked = true;
    }
  }

  if (actions == GLFW_RELEASE && !ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      is_left_mouse_clicked = false;
    }
    if (button == GLFW_MOUSE_BUTTON_2) {
      is_right_mouse_clicked = false;
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
      is_middle_mouse_clicked = false;
    }
  }


  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A2::mouseScrollEvent (
    double xOffSet,
    double yOffSet
) {
  bool eventHandled(false);

  zoom -= 2*yOffSet;
  t_proj = glm::perspective(
    glm::radians( zoom ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f, 1000.0f );

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A2::windowResizeEvent (
    int width,
    int height
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A2::keyInputEvent (
    int key,
    int action,
    int mods
) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

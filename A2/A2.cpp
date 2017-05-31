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
  is_changing_viewport = false;
  current_mode = 0;
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
  f_view.push_back(vec4(0.0f, 0.0f, -1.0f, 0.0f));
  f_view.push_back(vec4(0.0f, 0.0f, -5.0f, 1.0f));
}

//----------------------------------------------------------------------------------------
void A2::initializeModelCoordinates()
{
  // Initialize local coordinates for cube
  cube_model_coordinates.push_back( vec4(-1.0f, -1.0f, -1.0f, 1.0f) ); // left bottom front
  cube_model_coordinates.push_back( vec4(-1.0f, 1.0f, -1.0f, 1.0f) ); // left top front
  cube_model_coordinates.push_back( vec4(1.0f, -1.0f, -1.0f, 1.0f) ); // right bottom front
  cube_model_coordinates.push_back( vec4(1.0f, 1.0f, -1.0f, 1.0f) ); // right top front

  cube_model_coordinates.push_back( vec4(-1.0f, -1.0f, 1.0f, 1.0f) ); // left bottom back
  cube_model_coordinates.push_back( vec4(-1.0f, 1.0f, 1.0f, 1.0f) ); // left top back
  cube_model_coordinates.push_back( vec4(1.0f, -1.0f, 1.0f, 1.0f) ); // right bottom back
  cube_model_coordinates.push_back( vec4(1.0f, 1.0f, 1.0f, 1.0f) ); // right top back

  // Initiate the coordinates for the cube's gnomon
  cube_gnomon_model_coordinates.push_back( vec4(0.0f, 0.0f, 0.0f, 1.0f) );
  cube_gnomon_model_coordinates.push_back( vec4(0.25f, 0.0f, 0.0f, 1.0f) );
  cube_gnomon_model_coordinates.push_back( vec4(0.0f, 0.25f, 0.0f, 1.0f) );
  cube_gnomon_model_coordinates.push_back( vec4(0.0f, 0.0f, 0.25f, 1.0f) );

  // Initialize the coordinates for the world gnomon
  world_gnomon_model_coordinates.push_back( vec4(0.0f, 0.0f, 0.0f, 1.0f) );
  world_gnomon_model_coordinates.push_back( vec4(-0.8f, 0.0f, 0.0f, 1.0f) );
  world_gnomon_model_coordinates.push_back( vec4(0.0f, -0.8f, 0.0f, 1.0f) );
  world_gnomon_model_coordinates.push_back( vec4(0.0f, 0.0f, 0.8f, 1.0f) );

  // Initialize viewport coordinates
  viewport_xl = -0.95;
  viewport_yb = -0.95;
  viewport_xr = 0.95;
  viewport_yt = 0.95;
  width_ratio = (viewport_xr - viewport_xl)/2;
  height_ratio = (viewport_yt - viewport_yb)/2;
  left_bound = viewport_xl + (viewport_xr - viewport_xl)/2;
  bottom_bound = viewport_yb + (viewport_yt - viewport_yb)/2;

  // Initialize mode
  keys[GLFW_KEY_R] = true;
  current_mode = GLFW_KEY_R;
}

//----------------------------------------------------------------------------------------
void A2::initializeTransformationMatrices()
{
  t_model_cube = mat4(
    vec4( 0.5f, 0.0f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.5f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.5f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.0f, 1.0f )
  );

  t_model_cube_gnomon = mat4(
    vec4( 0.5f, 0.0f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.5f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.5f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.0f, 1.0f )
  );

  t_model_scale = mat4(
    vec4( 0.5f, 0.0f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.5f, 0.0f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.5f, 0.0f ),
    vec4( 0.0f, 0.0f, 0.0f, 1.0f )
  );
  t_model_rotation = mat4( 1.0f );
  t_model_translation = mat4( 1.0f );

  t_view = mat4( 1.0f );
  for (int i=0; i<4; i++) {
    for (int j=0; j<3; j++) {
      t_view[i][j] = dot( f_world[j], f_view[i] );
    }
    t_view[i][3] = dot( (f_world[3] - f_view[i]), f_view[i] );
  }

  near = 10.0f;
  far = 100.0f;
  fov = radians( 30.0f );
  t_proj = mat4(
    vec4( cos(fov/2)/sin(fov/2)/(m_windowHeight/m_windowWidth), 0.0f, 0.0f, 0.0f ),
    vec4( 0.0f, cos(fov/2)/sin(fov/2), 0.0f, 0.0f ),
    vec4( 0.0f, 0.0f, -(far + near)/(far - near), -1.0f ),
    vec4( 0.0f, 0.0f, (-2.0f * far * near)/(far - near), 0.0f )
  );
}

//----------------------------------------------------------------------------------------
void A2::applyModelTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  cube_world_coordinates.resize(0);
  cube_gnomon_world_coordinates.resize(0);
  world_gnomon_world_coordinates.resize(0);

  for (it=cube_model_coordinates.begin(); it!=cube_model_coordinates.end(); it++) {
    cube_world_coordinates.push_back(t_model_cube * (*it));
  }

  for (it=cube_gnomon_model_coordinates.begin(); it!=cube_gnomon_model_coordinates.end(); it++) {
    cube_gnomon_world_coordinates.push_back(t_model_cube_gnomon * (*it));
  }

  for (it=world_gnomon_model_coordinates.begin(); it!=world_gnomon_model_coordinates.end(); it++) {
    world_gnomon_world_coordinates.push_back(*it);
  }
}

//----------------------------------------------------------------------------------------
void A2::applyViewingTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  cube_view_coordinates.resize(0);
  cube_gnomon_view_coordinates.resize(0);
  world_gnomon_view_coordinates.resize(0);

  for (it=cube_world_coordinates.begin(); it!=cube_world_coordinates.end(); it++) {
    cube_view_coordinates.push_back(t_view * (*it));
  }

  for (it=cube_gnomon_world_coordinates.begin(); it!=cube_gnomon_world_coordinates.end(); it++) {
    cube_gnomon_view_coordinates.push_back(t_view * (*it));
  }

  for (it=world_gnomon_world_coordinates.begin(); it!=world_gnomon_world_coordinates.end(); it++) {
    world_gnomon_view_coordinates.push_back(t_view * (*it));
  }
}

//----------------------------------------------------------------------------------------
void A2::applyProjectionTransformation()
{
  // Apply transformations to the cube coordinates
  vector<vec4>::iterator it;
  cube_clip_coordinates.resize(0);
  cube_gnomon_clip_coordinates.resize(0);
  world_gnomon_normalized_device_coordinates.resize(0);

  for (it=cube_view_coordinates.begin(); it!=cube_view_coordinates.end(); it++) {
    cube_clip_coordinates.push_back(t_proj * (*it));
  }

  for (it=cube_gnomon_view_coordinates.begin(); it!=cube_gnomon_view_coordinates.end(); it++) {
    cube_gnomon_clip_coordinates.push_back(t_proj * (*it));
  }

  for (it=world_gnomon_view_coordinates.begin(); it!=world_gnomon_view_coordinates.end(); it++) {
    float x = (width_ratio * it->x/it->z) + left_bound;
    float y = (height_ratio * it->y/it->z) + bottom_bound;

    world_gnomon_normalized_device_coordinates.push_back(vec2(x, y));
  }
}

//----------------------------------------------------------------------------------------
void A2::buildAndClipLines()
{
  cube_lines.resize(0);
  cube_normalized_device_coordinates.resize(0);

  cube_lines.push_back(make_pair(cube_clip_coordinates[0], cube_clip_coordinates[1]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[2], cube_clip_coordinates[3]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[0], cube_clip_coordinates[2]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[1], cube_clip_coordinates[3]));

  cube_lines.push_back(make_pair(cube_clip_coordinates[4], cube_clip_coordinates[5]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[6], cube_clip_coordinates[7]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[4], cube_clip_coordinates[6]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[5], cube_clip_coordinates[7]));

  cube_lines.push_back(make_pair(cube_clip_coordinates[0], cube_clip_coordinates[4]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[2], cube_clip_coordinates[6]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[1], cube_clip_coordinates[5]));
  cube_lines.push_back(make_pair(cube_clip_coordinates[3], cube_clip_coordinates[7]));

  performClipping(cube_lines, &cube_normalized_device_coordinates);

  cube_gnomon_lines.resize(0);
  cube_gnomon_normalized_device_coordinates.resize(0);

  cube_gnomon_lines.push_back(make_pair(cube_gnomon_clip_coordinates[0], cube_gnomon_clip_coordinates[1]));
  cube_gnomon_lines.push_back(make_pair(cube_gnomon_clip_coordinates[0], cube_gnomon_clip_coordinates[2]));
  cube_gnomon_lines.push_back(make_pair(cube_gnomon_clip_coordinates[0], cube_gnomon_clip_coordinates[3]));

  performClipping(cube_gnomon_lines, &cube_gnomon_normalized_device_coordinates);
}


void A2::performClipping(vector< pair< vec4, vec4> > lines, vector<vec2> *ndcs)
{
  vector< pair< vec4, vec4> >::iterator it;
  for (it=lines.begin(); it!=lines.end(); it++) {
    // do clipping + normalization + viewport transformation
    bitset<4> outcode_c1 = generateOutCode(it->first);
    bitset<4> outcode_c2 = generateOutCode(it->second);

    if ((outcode_c1 | outcode_c2) == bitset<4>("0000")) {
      ndcs->push_back(normalizeVertex(it->first));
      ndcs->push_back(normalizeVertex(it->second));
    }
    else if ((outcode_c1 & outcode_c2) != bitset<4>("0000")) {
      continue;
    }
    else {
      // Full clip x plane
      if (outcode_c1[0]) {
        float a = (it->first.w + it->first.x) / (it->first.w - it->second.w + it->first.x - it->second.x);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->first = clipped;
      }
      else if (outcode_c2[0]) {
        float a = (it->first.w + it->first.x) / (it->first.w - it->second.w + it->first.x - it->second.x);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->second = clipped;
      }
      if (outcode_c1[1]) {
        float a = (it->first.w - it->first.x) / (it->first.w - it->second.w + it->second.x - it->first.x);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->first = clipped;
      }
      else if (outcode_c2[1]) {
        float a = (it->first.w - it->first.x) / (it->first.w - it->second.w + it->second.x - it->first.x);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->second = clipped;
      }

      // Full clip y plane
      if (outcode_c1[2]) {
        float a = (it->first.w + it->first.y) / (it->first.w - it->second.w + it->first.y - it->second.y);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->first = clipped;
      }
      else if (outcode_c2[2]) {
        float a = (it->first.w + it->first.y) / (it->first.w - it->second.w + it->first.y - it->second.y);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->second = clipped;
      }
      if (outcode_c1[3]) {
        float a = (it->first.w - it->first.y) / (it->first.w - it->second.w + it->second.y - it->first.y);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->first = clipped;
      }
      else if (outcode_c2[3]) {
        float a = (it->first.w - it->first.y) / (it->first.w - it->second.w + it->second.y - it->first.y);
        vec4 clipped = (1-a)*it->first + a*it->second;
        it->second = clipped;
      }

      ndcs->push_back(normalizeVertex(it->first));
      ndcs->push_back(normalizeVertex(it->second));
    }
  }
}

//----------------------------------------------------------------------------------------
bitset<4> A2::generateOutCode(vec4 p)
{
  bitset<4> outcode;

  if (p.w + p.x < 0) {
    outcode[0] = true;
  }
  if (p.w - p.x < 0) {
    outcode[1] = true;
  }
  if (p.w + p.y < 0) {
    outcode[2] = true;
  }
  if (p.w - p.y < 0) {
    outcode[3] = true;
  }
  // if (p.w + p.z < 0) {
  //   outcode[4] = true;
  // }
  // if (p.w - p.z < 0) {
  //   outcode[5] = true;
  // }

  return outcode;
}

//---------------------------------------------------------------------------------------
vec2 A2::normalizeVertex(vec4 p)
{
  float x = (width_ratio * p.x/p.w) + left_bound;
  float y = (height_ratio * p.y/p.w) + bottom_bound;

  return vec2(x, y);
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
  vector<vec2>::iterator itv2;
  applyModelTransformation();
  applyViewingTransformation();
  applyProjectionTransformation();
  buildAndClipLines();

  // Call at the beginning of frame, before drawing lines:
  initLineData();

  // // Draw viewport:
  setLineColour(vec3(0.2f, 1.0f, 1.0f));
  drawLine(vec2(viewport_xl, viewport_yb), vec2(viewport_xr, viewport_yb));
  drawLine(vec2(viewport_xr, viewport_yb), vec2(viewport_xr, viewport_yt));
  drawLine(vec2(viewport_xr, viewport_yt), vec2(viewport_xl, viewport_yt));
  drawLine(vec2(viewport_xl, viewport_yt), vec2(viewport_xl, viewport_yb));

  setLineColour(vec3(1.0f, 0.7f, 0.8f));
  for (itv2=cube_normalized_device_coordinates.begin(); itv2!=cube_normalized_device_coordinates.end(); itv2+=2) {
    drawLine(*itv2, *(itv2+1));
  }

  // Draw the cube's local gnomon
  setLineColour(vec3(0.2f, 1.0f, 1.0f));
  for (itv2=cube_gnomon_normalized_device_coordinates.begin(); itv2!=cube_gnomon_normalized_device_coordinates.end(); itv2+=2) {
    drawLine(*itv2, *(itv2+1));
  }

  // Draw the world gnomon
  setLineColour(vec3(0.0f, 0.0f, 0.0f));
  drawLine(world_gnomon_normalized_device_coordinates[0], world_gnomon_normalized_device_coordinates[1]);
  drawLine(world_gnomon_normalized_device_coordinates[0], world_gnomon_normalized_device_coordinates[2]);
  drawLine(world_gnomon_normalized_device_coordinates[0], world_gnomon_normalized_device_coordinates[3]);
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
    ImGui::Text( "Near %.1f", near );
    ImGui::Text( "Far %.1f", far );
    ImGui::Text( "Fov %.1f", degrees(fov) );

    ImGui::RadioButton( "Rotate Model", &current_mode, GLFW_KEY_R );
    ImGui::RadioButton( "Translate Model", &current_mode, GLFW_KEY_T );
    ImGui::RadioButton( "Scale Model", &current_mode, GLFW_KEY_S );

    ImGui::RadioButton( "Rotate View", &current_mode, GLFW_KEY_O );
    ImGui::RadioButton( "Translate View", &current_mode, GLFW_KEY_N );
    ImGui::RadioButton( "Perspective", &current_mode, GLFW_KEY_P );

    ImGui::RadioButton( "Viewport Mode", &current_mode, GLFW_KEY_V );

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

  // Camera rotation
  if (current_mode == GLFW_KEY_O) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.25 * M_PI;
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, cos(q), sin(q), 0.0f ),
        vec4( 0.0f, -sin(q), cos(q), 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_view = inverse(transform) * t_view;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.25 * M_PI;
      mat4 transform = mat4(
        vec4( cos(q), 0.0f, -sin(q), 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( sin(q), 0.0f, cos(q), 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_view = inverse(transform) * t_view;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.25 * M_PI;
      mat4 transform = mat4(
        vec4( cos(q), sin(q), 0.0f, 0.0f ),
        vec4( -sin(q), cos(q), 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_view = inverse(transform) * t_view;
    }
  }

  // Camera translation
  if (current_mode == GLFW_KEY_N) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = (xPos - mouse_x_pos) / (m_windowWidth*10);
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0 ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( q, 0.0f, 0.0f, 1.0f )
      );
       t_view = inverse(transform) * t_view;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = (xPos - mouse_x_pos) / (m_windowWidth*10);
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, q, 0.0f, 1.0f )
      );
      t_view = inverse(transform) * t_view;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = (xPos - mouse_x_pos) / (m_windowWidth*10);
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, q, 1.0f )
      );
       t_view = inverse(transform) * t_view;
    }
  }

  // Perspective transformation
  if (current_mode == GLFW_KEY_P) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * M_PI;
      if (degrees(fov + q) < 150 && degrees(fov + q) > 5) {
        fov += q;
        t_proj = mat4(
          vec4( cos(fov/2)/sin(fov/2)/(m_windowWidth/m_windowHeight), 0.0f, 0.0f, 0.0f ),
          vec4( 0.0f, cos(fov/2)/sin(fov/2), 0.0f, 0.0f ),
          vec4( 0.0f, 0.0f, -(far + near)/(far - near), -1.0f ),
          vec4( 0.0f, 0.0f, (-2.0f * far * near)/(far - near), 0.0f )
        );
      }
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 10;
      if (far + q > near) {
        far += q;
        t_proj = mat4(
          vec4( cos(fov/2)/sin(fov/2)/(m_windowWidth/m_windowHeight), 0.0f, 0.0f, 0.0f ),
          vec4( 0.0f, cos(fov/2)/sin(fov/2), 0.0f, 0.0f ),
          vec4( 0.0f, 0.0f, -(far + near)/(far - near), -1.0f ),
          vec4( 0.0f, 0.0f, (-2.0f * far * near)/(far - near), 0.0f )
        );
      }
    }
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 10;
      if (near + q > 0) {
        near += q;
        t_proj = mat4(
          vec4( cos(fov/2)/sin(fov/2)/(m_windowWidth/m_windowHeight), 0.0f, 0.0f, 0.0f ),
          vec4( 0.0f, cos(fov/2)/sin(fov/2), 0.0f, 0.0f ),
          vec4( 0.0f, 0.0f, -(far + near)/(far - near), -1.0f ),
          vec4( 0.0f, 0.0f, (-2.0f * far * near)/(far - near), 0.0f )
        );
      }
    }
  }

  // Model rotation
  if (current_mode == GLFW_KEY_R) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.5 * M_PI;
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, cos(q), sin(q), 0.0f ),
        vec4( 0.0f, -sin(q), cos(q), 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_rotation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.5 * M_PI;
      mat4 transform = mat4(
        vec4( cos(q), 0.0f, -sin(q), 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( sin(q), 0.0f, cos(q), 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_rotation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = ((xPos - mouse_x_pos) / m_windowWidth) * 0.5 * M_PI;
      mat4 transform = mat4(
        vec4( cos(q), sin(q), 0.0f, 0.0f ),
        vec4( -sin(q), cos(q), 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_rotation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }
  }

  // Model translation
  if (current_mode == GLFW_KEY_T) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = (xPos - mouse_x_pos) / m_windowWidth * 10;
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0 ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( q, 0.0f, 0.0f, 1.0f )
      );
      t_model_translation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = (xPos - mouse_x_pos) / m_windowWidth * 10;
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, q, 0.0f, 1.0f )
      );
      t_model_translation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = (xPos - mouse_x_pos) / m_windowWidth * 10;
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, q, 1.0f )
      );
      t_model_translation *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
      t_model_cube_gnomon = t_model_translation * t_model_rotation;
    }
  }

  // Model scaling
  if (current_mode == GLFW_KEY_S) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      float q = (xPos - mouse_x_pos);
      if (q < 0) {
        q = 0.99f;
      }
      else {
        q = 1.01f;
      }
      mat4 transform = mat4(
        vec4( q, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_scale *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_2]) {
      float q = (xPos - mouse_x_pos);
      if (q < 0) {
        q = 0.99f;
      }
      else {
        q = 1.01f;
      }
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, q, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 1.0f, 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_scale *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
    }

    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_3]) {
      float q = (xPos - mouse_x_pos);
      if (q < 0) {
        q = 0.99f;
      }
      else {
        q = 1.01f;
      }
      mat4 transform = mat4(
        vec4( 1.0f, 0.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 1.0f, 0.0f, 0.0f ),
        vec4( 0.0f, 0.0f, q, 0.0f ),
        vec4( 0.0f, 0.0f, 0.0f, 1.0f )
      );
      t_model_scale *= transform;
      t_model_cube = t_model_translation * t_model_rotation * t_model_scale;
    }
  }

  if (current_mode == GLFW_KEY_V) {
    if (!ImGui::IsMouseHoveringAnyWindow() && keys[GLFW_MOUSE_BUTTON_1]) {
      if (!is_changing_viewport) {
        is_changing_viewport = true;
        viewport_xl = (xPos/m_windowWidth)*2 - 1;
        viewport_yb = -(yPos/m_windowHeight)*2 + 1;

      }
      else {
        viewport_xr = (xPos/m_windowWidth)*2 - 1;
        viewport_yt = -(yPos/m_windowHeight)*2 + 1;
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
bool A2::mouseButtonInputEvent (
    int button,
    int actions,
    int mods
) {
  bool eventHandled(false);

  if (actions == GLFW_PRESS && !ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      keys[GLFW_MOUSE_BUTTON_1] = true;
    }
    if (button == GLFW_MOUSE_BUTTON_2) {
      keys[GLFW_MOUSE_BUTTON_2] = true;
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
      keys[GLFW_MOUSE_BUTTON_3] = true;
    }
  }

  if (actions == GLFW_RELEASE && !ImGui::IsMouseHoveringAnyWindow()) {
    if (button == GLFW_MOUSE_BUTTON_1) {
      keys[GLFW_MOUSE_BUTTON_1] = false;

      // Tidy up viewport mode and coordinates
      if (current_mode == GLFW_KEY_V) {
        is_changing_viewport = false;
        if (viewport_yt < viewport_yb) {
          swap(viewport_yt, viewport_yb);
        }
        if (viewport_xr < viewport_xl) {
          swap(viewport_xl, viewport_xr);
        }
        width_ratio = (viewport_xr - viewport_xl)/2;
        height_ratio = (viewport_yt - viewport_yb)/2;
        left_bound = viewport_xl + (viewport_xr - viewport_xl)/2;
        bottom_bound = viewport_yb + (viewport_yt - viewport_yb)/2;
      }
    }
    if (button == GLFW_MOUSE_BUTTON_2) {
      keys[GLFW_MOUSE_BUTTON_2] = false;
    }
    if (button == GLFW_MOUSE_BUTTON_3) {
      keys[GLFW_MOUSE_BUTTON_3] = false;
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
  bool eventHandled(true);

  if (action == GLFW_PRESS) {
    current_mode = key;

    if (key == GLFW_KEY_A) {
      initializeCoordinateFrames();
      initializeModelCoordinates();
      initializeTransformationMatrices();
    }
    else if (key == GLFW_KEY_Q) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }
  }

  return eventHandled;
}

#include "A1.hpp"
#include "cs488-framework/GlErrorCheck.hpp"

#include <iostream>
#include <algorithm>

#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;
using namespace std;

static const size_t DIM = 16;

//----------------------------------------------------------------------------------------
// Constructor
A1::A1()
  : current_column( 0 ),
    current_row( 0 ),
    current_col( 0 ),
    cubes( DIM*DIM, vec3(0,0,0) ),
    cube_indices( DIM*DIM, vector<unsigned int>(0, 0) ),
    flattened_cube_indices( 0, 0 ),
    indicator( 0, vec3(0,0,0) ),
    zoom( 45.0f ),
    is_shift_pressed( false )
{
  colour[0] = 0.0f;
  colour[1] = 0.0f;
  colour[2] = 0.0f;

  colour[3] = 1.0f;
  colour[4] = 0.0f;
  colour[5] = 0.0f;

  colour[6] = 0.0f;
  colour[7] = 1.0f;
  colour[8] = 0.0f;

  colour[9] = 0.0f;
  colour[10] = 0.0f;
  colour[11] = 1.0f;

  colour[12] = 0.4f;
  colour[13] = 0.4f;
  colour[14] = 0.4f;

  colour[15] = 1.0f;
  colour[16] = 1.0f;
  colour[17] = 1.0f;

  colour[18] = 0.7f;
  colour[19] = 0.3f;
  colour[20] = 0.7f;

  colour[21] = 0.3f;
  colour[22] = 0.7f;
  colour[23] = 0.3f;
}

//----------------------------------------------------------------------------------------
// Destructor
A1::~A1()
{}

//----------------------------------------------------------------------------------------
/*
 * Called once, at program start.
 */
void A1::init()
{
  // Set the background colour.
  glClearColor( 0.3, 0.5, 0.7, 1.0 );

  // Build the shader
  m_shader.generateProgramObject();
  m_shader.attachVertexShader(
    getAssetFilePath( "VertexShader.vs" ).c_str() );
  m_shader.attachFragmentShader(
    getAssetFilePath( "FragmentShader.fs" ).c_str() );
  m_shader.link();

  // Set up the uniforms
  P_uni = m_shader.getUniformLocation( "P" );
  V_uni = m_shader.getUniformLocation( "V" );
  M_uni = m_shader.getUniformLocation( "M" );
  col_uni = m_shader.getUniformLocation( "colour" );

  initGrid();

  // Set up initial view and projection matrices (need to do this here,
  // since it depends on the GLFW window being set up correctly).
  view = glm::lookAt(
    glm::vec3( 0.0f, float(DIM)*2.0*M_SQRT1_2, float(DIM)*2.0*M_SQRT1_2 ),
    glm::vec3( 0.0f, 0.0f, 0.0f ),
    glm::vec3( 0.0f, 1.0f, 0.0f ) );

  proj = glm::perspective(
    glm::radians( zoom ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f, 1000.0f );
}

void A1::initGrid()
{
  size_t sz = 3 * 2 * 2 * (DIM+3);

  float *verts = new float[ sz ];
  size_t ct = 0;
  for( int idx = 0; idx < DIM+3; ++idx ) {
    // Vertical line segments
    verts[ ct ] = -1;
    verts[ ct+1 ] = 0;
    verts[ ct+2 ] = idx-1;
    verts[ ct+3 ] = DIM+1;
    verts[ ct+4 ] = 0;
    verts[ ct+5 ] = idx-1;
    ct += 6;

    // Horizontal line segments
    verts[ ct ] = idx-1;
    verts[ ct+1 ] = 0;
    verts[ ct+2 ] = -1;
    verts[ ct+3 ] = idx-1;
    verts[ ct+4 ] = 0;
    verts[ ct+5 ] = DIM+1;
    ct += 6;
  }

  // VAO for grid.
  glGenVertexArrays( 1, &m_grid_vao );
  glBindVertexArray( m_grid_vao );

  // VBO for grid.
  glGenBuffers( 1, &m_grid_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_grid_vbo );
  glBufferData( GL_ARRAY_BUFFER, sz*sizeof(float),
    verts, GL_STATIC_DRAW );

  GLint gridPosAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( gridPosAttrib );
  glVertexAttribPointer( gridPosAttrib, 3, GL_FLOAT, GL_FALSE, 0, nullptr );


  // Indicator vertices
  vector<vec3> indicatorVerts = {
    vec3( current_column, 0, current_row ),
    vec3( current_column, 0, current_row+1 ),
    vec3( current_column+1, 0, current_row+1 ),
    vec3( current_column, 0, current_row ),
    vec3( current_column+1, 0, current_row ),
    vec3( current_column+1, 0, current_row+1 ),
  };

  indicator.insert(indicator.begin(), indicatorVerts.begin(), indicatorVerts.end());

  // VAO for indicator.
  glGenVertexArrays( 1, &m_indicator_vao );
  glBindVertexArray( m_indicator_vao );

  // VBO for indicator
  glGenBuffers( 1, &m_indicator_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_indicator_vbo );
  glBufferData( GL_ARRAY_BUFFER, indicator.size()*sizeof(vec3),
    &indicator[0], GL_DYNAMIC_DRAW );

  // Specify the means of extracting the position values properly.
  GLint indicatorPosAttrib = m_shader.getAttribLocation( "position" );
  glEnableVertexAttribArray( indicatorPosAttrib );
  glVertexAttribPointer( indicatorPosAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), nullptr );


  // Generate the vertex buffers for building cubes
  glGenVertexArrays( 1, &m_cubes_vao );
  glBindVertexArray( m_cubes_vao );

  glGenBuffers( 1, &m_cubes_vbo );
  glBindBuffer( GL_ARRAY_BUFFER, m_cubes_vbo );

  {
    GLint cubesAttrib = m_shader.getAttribLocation( "position" );
    glEnableVertexAttribArray( cubesAttrib );
    GLsizei stride = sizeof(vec3);
    const GLvoid * offset = 0;
    glVertexAttribPointer( cubesAttrib, 3, GL_FLOAT, GL_FALSE, stride, nullptr );
  }

  {
    GLint cubesColorAttrib = m_shader.getUniformLocation( "colour" );
    glEnableVertexAttribArray( cubesColorAttrib );
  }

  // Generate the index buffer
  glGenBuffers( 1, &m_cubes_index_vbo );

  // Reset state to prevent rogue code from messing with *my*
  // stuff!
  glBindVertexArray( 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  // OpenGL has the buffer now, there's no need for us to keep a copy.
  delete [] verts;

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Updates the position of the indicator.
 */
void A1::updateIndicatorPos()
{
  // Indicator vertices
  vector<vec3> indicatorVerts = {
    vec3( current_column, 0, current_row ),
    vec3( current_column, 0, current_row+1 ),
    vec3( current_column+1, 0, current_row+1 ),
    vec3( current_column, 0, current_row ),
    vec3( current_column+1, 0, current_row ),
    vec3( current_column+1, 0, current_row+1 ),
  };

  indicator.clear();
  indicator.insert(indicator.begin(), indicatorVerts.begin(), indicatorVerts.end());

  glBindBuffer( GL_ARRAY_BUFFER, m_indicator_vbo );
  glBufferData( GL_ARRAY_BUFFER, indicator.size()*sizeof(vec3),
    &indicator[0], GL_DYNAMIC_DRAW );
}

void A1::copyStack(int prev_col, int prev_row)
{
  int prev_height = cube_indices[prev_col+prev_row*DIM].size();
  int height = cube_indices[current_column+current_row*DIM].size();
  int diff = (prev_height - height) / 30;

  if (diff == 0) {
    return;
  }
  else if (diff < 0) {
    removeCube(diff);
  }
  else {
    for (int i=0; i<diff; i++) {
      addCube();
    }
  }
}

void A1::flattenCubeIndices()
{
  flattened_cube_indices.clear();

  for (int i=0; i<cube_indices.size(); i++) {
    for (int j=0; j<cube_indices[i].size(); j++) {
      flattened_cube_indices.push_back(cube_indices[i][j]);
    }
  }
}

//----------------------------------------------------------------------------------------
/*
 * Defines the vertices for a cube at posy, posx and posz.
 */
void A1::addCube()
{
  int posx = current_column;
  int posz = current_row;
  int posy = cube_indices[current_column + current_row*DIM].size() / 30;
  unsigned int ct = cubes.size();

  vector<vec3> new_cubes = {
    vec3( posx, posy, posz ),
    vec3( posx, posy+1, posz ),
    vec3( posx+1, posy+1, posz ),
    vec3( posx+1, posy, posz ),
    vec3( posx+1, posy+1, posz+1 ),
    vec3( posx+1, posy, posz+1 ),
    vec3( posx, posy+1, posz+1 ),
    vec3( posx, posy, posz+1 )
  };

  cubes.insert( cubes.end(), new_cubes.begin(), new_cubes.end() );
  glBindBuffer( GL_ARRAY_BUFFER, m_cubes_vbo );
  glBufferData( GL_ARRAY_BUFFER, cubes.size()*sizeof(vec3),
    &cubes[0], GL_DYNAMIC_DRAW );

  vector<unsigned int> new_indices =
  {
    ct + 0, ct + 1, ct + 2,
    ct + 0, ct + 2, ct + 3,
    ct + 2, ct + 3, ct + 5,
    ct + 2, ct + 4, ct + 5,
    ct + 4, ct + 5, ct + 6,
    ct + 5, ct + 6, ct + 7,
    ct + 0, ct + 1, ct + 6,
    ct + 0, ct + 6, ct + 7,
    ct + 1, ct + 2, ct + 4,
    ct + 1, ct + 4, ct + 6
  };

  cube_indices[current_column+current_row*DIM].insert( cube_indices[current_column+current_row*DIM].end(), new_indices.begin(), new_indices.end() );
  flattenCubeIndices();
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cubes_index_vbo );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, flattened_cube_indices.size()*sizeof(GL_UNSIGNED_INT),
    &flattened_cube_indices[0], GL_DYNAMIC_DRAW);


  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );

}

void A1::removeCube(int num)
{
  cube_indices[current_column+current_row*DIM].resize(cube_indices[current_column+current_row*DIM].size() - 30*num);
  flattenCubeIndices();
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cubes_index_vbo );
  glBufferData( GL_ELEMENT_ARRAY_BUFFER, flattened_cube_indices.size()*sizeof(GL_UNSIGNED_INT),
    &flattened_cube_indices[0], GL_DYNAMIC_DRAW);

  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
  glBindBuffer( GL_ARRAY_BUFFER, 0 );
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void A1::appLogic()
{
  // Place per frame, application logic here ...
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void A1::guiLogic()
{
  // We already know there's only going to be one window, so for
  // simplicity we'll store button states in static local variables.
  // If there was ever a possibility of having multiple instances of
  // A1 running simultaneously, this would break; you'd want to make
  // this into instance fields of A1.
  static bool showTestWindow(false);
  static bool showDebugWindow(true);

  ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize);
  float opacity(0.5f);

  ImGui::Begin("Debug Window", &showDebugWindow, ImVec2(100,100), opacity, windowFlags);
    if( ImGui::Button( "Quit Application" ) ) {
      glfwSetWindowShouldClose(m_window, GL_TRUE);
    }

    // Eventually you'll create multiple colour widgets with
    // radio buttons.  If you use PushID/PopID to give them all
    // unique IDs, then ImGui will be able to keep them separate.
    // This is unnecessary with a single colour selector and
    // radio button, but I'm leaving it in as an example.

    // Prefixing a widget name with "##" keeps it from being
    // displayed.

    ImGui::ColorEdit3( "##Colour0", colour );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col0", &current_col, 0 );

    ImGui::ColorEdit3( "##Colour1", &colour[3] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col1", &current_col, 1 );

    ImGui::ColorEdit3( "##Colour2", &colour[6] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col2", &current_col, 2 );

    ImGui::ColorEdit3( "##Colour3", &colour[9] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col3", &current_col, 3 );

    ImGui::ColorEdit3( "##Colour4", &colour[12] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col4", &current_col, 4 );

    ImGui::ColorEdit3( "##Colour5", &colour[15] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col5", &current_col, 5 );

    ImGui::ColorEdit3( "##Colour6", &colour[18] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col6", &current_col, 6 );

    ImGui::ColorEdit3( "##Colour7", &colour[21] );
    ImGui::SameLine();
    ImGui::RadioButton( "##Col7", &current_col, 7 );


    // For convenience, you can uncomment this to show ImGui's massive
    // demonstration window right in your application.  Very handy for
    // browsing around to get the widget you want.  Then look in
    // shared/imgui/imgui_demo.cpp to see how it's done.
    if( ImGui::Button( "Test Window" ) ) {
      showTestWindow = !showTestWindow;
    }


    ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

  ImGui::End();

  if( showTestWindow ) {
    ImGui::ShowTestWindow( &showTestWindow );
  }
}

//----------------------------------------------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void A1::draw()
{
  // Create a global transformation for the model (centre it).
  mat4 W;
  W = glm::translate( W, vec3( -float(DIM)/2.0f, 0, -float(DIM)/2.0f ) );

  m_shader.enable();
    glEnable( GL_DEPTH_TEST );

    glUniformMatrix4fv( P_uni, 1, GL_FALSE, value_ptr( proj ) );
    glUniformMatrix4fv( V_uni, 1, GL_FALSE, value_ptr( view ) );
    glUniformMatrix4fv( M_uni, 1, GL_FALSE, value_ptr( W ) );

    // Just draw the grid for now.
    glBindVertexArray( m_grid_vao );
    glUniform3f( col_uni, 1, 1, 1 );
    glDrawArrays( GL_LINES, 0, (3+DIM)*4);

    // Draw the cubes
    glBindVertexArray( m_cubes_vao );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_cubes_index_vbo );
    glUniform3f( col_uni, colour[current_col*3], colour[current_col*3+1], colour[current_col*3+2] );
    glDrawElements( GL_TRIANGLES, flattened_cube_indices.size(), GL_UNSIGNED_INT, nullptr );

    // Highlight the active square.
    glDisable(GL_DEPTH_TEST);
    glBindVertexArray( m_indicator_vao );
    glUniform3f( col_uni, 0.5, 0.5, 0.5 );
    glDrawArrays( GL_TRIANGLES, 0, indicator.size() );
  m_shader.disable();

  // Restore defaults
  glBindVertexArray( 0 );
  glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

  CHECK_GL_ERRORS;
}

//----------------------------------------------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void A1::cleanup()
{}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool A1::cursorEnterWindowEvent (
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
bool A1::mouseMoveEvent(double xPos, double yPos)
{
  bool eventHandled(false);

  if (!ImGui::IsMouseHoveringAnyWindow()) {
    // Put some code here to handle rotations.  Probably need to
    // check whether we're *dragging*, not just moving the mouse.
    // Probably need some instance variables to track the current
    // rotation amount, and maybe the previous X position (so
    // that you can rotate relative to the *change* in X.
  }

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool A1::mouseButtonInputEvent(int button, int actions, int mods) {
  bool eventHandled(false);

  if (!ImGui::IsMouseHoveringAnyWindow()) {
    // The user clicked in the window.  If it's the left
    // mouse button, initiate a rotation.
  }

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool A1::mouseScrollEvent(double xOffSet, double yOffSet) {
  bool eventHandled(false);

  zoom -= 2*yOffSet;
  proj = glm::perspective(
    glm::radians( zoom ),
    float( m_framebufferWidth ) / float( m_framebufferHeight ),
    1.0f, 1000.0f );

  return eventHandled;
}

//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool A1::windowResizeEvent(int width, int height) {
  bool eventHandled(false);

  // Fill in with event handling code...

  return eventHandled;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool A1::keyInputEvent(int key, int action, int mods) {
  bool eventHandled(false);

  if (action == GLFW_PRESS) {
    if (key == GLFW_KEY_SPACE) {
      addCube();
    }
    if (key == GLFW_KEY_BACKSPACE) {
      removeCube(1);
    }
    if (key == GLFW_KEY_DOWN && current_row < DIM) {
      current_row++;
      updateIndicatorPos();
      if (is_shift_pressed) {
        copyStack(current_column, current_row-1);
      }
    }
    if (key == GLFW_KEY_UP && current_row > 0) {
      current_row--;
      updateIndicatorPos();
      if (is_shift_pressed) {
        copyStack(current_column, current_row+1);
      }
    }
    if (key == GLFW_KEY_RIGHT && current_column < DIM) {
      current_column++;
      updateIndicatorPos();
      if (is_shift_pressed) {
        copyStack(current_column-1, current_row);
      }
    }
    if (key == GLFW_KEY_LEFT && current_column > 0) {
      current_column--;
      updateIndicatorPos();
      if (is_shift_pressed) {
        copyStack(current_column+1, current_row);
      }
    }
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
      is_shift_pressed = true;
    }
  }
  if (action == GLFW_RELEASE) {
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
      is_shift_pressed = false;
    }
  }
}

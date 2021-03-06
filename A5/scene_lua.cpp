//
// CS488 - Introduction to Computer Graphics
//
// scene_lua.cpp
//
// Everything that's needed to parse a scene file using Lua.
// You don't necessarily have to understand exactly everything that
// goes on here, although it will be useful to have a reasonable idea
// if you wish to add new commands to the scene format.
//
// Lua interfaces with C/C++ using a special stack. Everytime you want
// to get something from lua, or pass something back to lua (e.g. a
// return value), you need to use this stack. Thus, most of the lua_
// and luaL_ functions actually manipulate the stack. All the
// functions beginning with "lua_" are part of the Lua C API itself,
// whereas the "luaL_" functions belong to a library of useful
// functions on top of that called lauxlib.
//
// This file consists of a bunch of C function declarations which
// implement functions callable from Lua. There are also two tables
// used to set up the interface between Lua and these functions, and
// the main "driver" function, import_lua, which calls the lua
// interpreter and sets up all the state.
//
// Note that each of the function declarations follow the same format:
// they take as their only argument the current state of the lua
// interpreter, and return the number of values returned back to lua.
//
// For more information see the book "Programming In Lua," available
// online at http://www.lua.org/pil/, and of course the Lua reference
// manual at http://www.lua.org/manual/5.0/.
//
// http://lua-users.org/wiki/LauxLibDocumentation provides a useful
// documentation of the "lauxlib" functions (beginning with luaL_).
//
// -- University of Waterloo Computer Graphics Lab 2005

#include "scene_lua.hpp"
#include <iostream>
#include <cctype>
#include <cstring>
#include <cstdio>
#include "lua488.hpp"
#include "AnimationNode.hpp"
#include "JointNode.hpp"
#include "GeometryNode.hpp"
#include "PhysicsNode.hpp"
#include "Primitive.hpp"

// Uncomment the following line to enable debugging messages
//#define GRLUA_ENABLE_DEBUG

#ifdef GRLUA_ENABLE_DEBUG
#  define GRLUA_DEBUG(x) do { std::cerr << x << std::endl; } while (0)
#  define GRLUA_DEBUG_CALL do { std::cerr << __FUNCTION__ << std::endl; } while (0)
#else
#  define GRLUA_DEBUG(x) do { } while (0)
#  define GRLUA_DEBUG_CALL do { } while (0)
#endif

// You may wonder, for the following types, why we use special "_ud"
// types instead of, for example, just allocating SceneNodes directly
// from lua. Part of the answer is that Lua is a C api. It doesn't
// call any constructors or destructors for you, so it's easier if we
// let it just allocate a pointer for the node, and handle
// allocation/deallocation of the node itself. Another (perhaps more
// important) reason is that we will want SceneNodes to stick around
// even after lua is done with them, after all, we want to pass them
// back to the program. If we let Lua allocate SceneNodes directly,
// we'd lose them all when we are done parsing the script. This way,
// we can easily keep around the data, all we lose is the extra
// pointers to it.

// The "userdata" type for a node. Objects of this type will be
// allocated by Lua to represent nodes.
struct gr_node_ud {
  SceneNode* node;
};

// The "userdata" type for a material. Objects of this type will be
// allocated by Lua to represent materials.
struct gr_material_ud {
  Material* material;
};

// Create a node
extern "C"
int gr_node_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)lua_newuserdata(L, sizeof(gr_node_ud));
  data->node = 0;

  const char* name = luaL_checkstring(L, 1);
  data->node = new SceneNode(name);

  luaL_getmetatable(L, "gr.node");
  lua_setmetatable(L, -2);

  return 1;
}

// Create a joint node
extern "C"
int gr_joint_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)lua_newuserdata(L, sizeof(gr_node_ud));
  data->node = 0;

  const char* name = luaL_checkstring(L, 1);
  JointNode* node = new JointNode(name);

  luaL_checktype(L, 2, LUA_TTABLE);

  luaL_argcheck(L, luaL_len(L, 2) == 3, 2, "Three-tuple expected");

  luaL_checktype(L, 3, LUA_TTABLE);

  luaL_argcheck(L, luaL_len(L, 3) == 3, 3, "Three-tuple expected");

  double x[3], y[3];
  for (int i = 1; i <= 3; i++) {
    lua_rawgeti(L, 2, i);
    x[i - 1] = luaL_checknumber(L, -1);
    lua_rawgeti(L, 3, i);
    y[i - 1] = luaL_checknumber(L, -1);
    lua_pop(L, 2);
  }

  node->set_joint_x(x[0], x[1], x[2]);
  node->set_joint_y(y[0], y[1], y[2]);

  data->node = node;

  luaL_getmetatable(L, "gr.node");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C"
int gr_physics_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)lua_newuserdata(L, sizeof(gr_node_ud));
  data->node = 0;

  const char* name = luaL_checkstring(L, 1);
  const char* prim_type = luaL_checkstring(L, 2);
  double x_pos = luaL_checknumber(L, 3);
  double y_pos = luaL_checknumber(L, 4);
  double z_pos = luaL_checknumber(L, 5);
  double x_size = luaL_checknumber(L, 6);
  double y_size = luaL_checknumber(L, 7);
  double z_size = luaL_checknumber(L, 8);
  unsigned int object_type = luaL_checknumber(L, 9);

  Primitive *prim = new Primitive(
    prim_type,
    glm::vec3(x_pos, y_pos, z_pos),
    glm::vec3(x_size, y_size, z_size)
  );

  data->node = new PhysicsNode(name, prim, object_type);

  luaL_getmetatable(L, "gr.node");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C"
int gr_animation_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)lua_newuserdata(L, sizeof(gr_node_ud));
  data->node = 0;

  const char* name = luaL_checkstring(L, 1);
  const char* prim_type = luaL_checkstring(L, 2);
  double x_pos = luaL_checknumber(L, 3);
  double y_pos = luaL_checknumber(L, 4);
  double z_pos = luaL_checknumber(L, 5);
  double x_size = luaL_checknumber(L, 6);
  double y_size = luaL_checknumber(L, 7);
  double z_size = luaL_checknumber(L, 8);
  unsigned int object_type = luaL_checknumber(L, 9);

  Primitive *prim = new Primitive(
    prim_type,
    glm::vec3(x_pos, y_pos, z_pos),
    glm::vec3(x_size, y_size, z_size)
  );

  data->node = new AnimationNode(name, prim, object_type);

  luaL_getmetatable(L, "gr.node");
  lua_setmetatable(L, -2);

  return 1;
}

extern "C"
int gr_mesh_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)lua_newuserdata(L, sizeof(gr_node_ud));
  data->node = 0;

  const char* meshId = luaL_checkstring(L, 1);
  const char* name = luaL_checkstring(L, 2);
  unsigned int texture = luaL_checknumber(L, 3);

  data->node = new GeometryNode(meshId, name, texture);

  luaL_getmetatable(L, "gr.node");
  lua_setmetatable(L, -2);

  return 1;
}


// Create a material
extern "C"
int gr_material_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_material_ud* data = (gr_material_ud*)lua_newuserdata(L, sizeof(gr_material_ud));
  data->material = 0;

  luaL_checktype(L, 1, LUA_TTABLE);

  luaL_argcheck(L, luaL_len(L, 1) == 3, 1, "Three-tuple expected");

  luaL_checktype(L, 2, LUA_TTABLE);

  luaL_argcheck(L, luaL_len(L, 2) == 3, 2, "Three-tuple expected");

  luaL_checktype(L, 3, LUA_TNUMBER);

  double kd[3], ks[3];
  for (int i = 1; i <= 3; i++) {
    lua_rawgeti(L, 1, i);
    kd[i - 1] = luaL_checknumber(L, -1);
    lua_rawgeti(L, 2, i);
    ks[i - 1] = luaL_checknumber(L, -1);
    lua_pop(L, 2);
  }
  double shininess = luaL_checknumber(L, 3);

  data->material = new Material();
  for(int i(0); i < 3; ++i) {
    data->material->kd[i] = kd[i];
    data->material->ks[i] = ks[i];
  }
  data->material->shininess = shininess;

  luaL_newmetatable(L, "gr.material");
  lua_setmetatable(L, -2);

  return 1;
}

// Add a child to a node
extern "C"
int gr_node_add_child_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  SceneNode* self = selfdata->node;

  gr_node_ud* childdata = (gr_node_ud*)luaL_checkudata(L, 2, "gr.node");
  luaL_argcheck(L, childdata != 0, 2, "Node expected");

  SceneNode* child = childdata->node;

  self->add_child(child);

  return 0;
}

// Set a node's material
extern "C"
int gr_node_set_material_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  GeometryNode* self = dynamic_cast<GeometryNode*>(selfdata->node);

  luaL_argcheck(L, self != 0, 1, "Geometry node expected");

  gr_material_ud* matdata = (gr_material_ud*)luaL_checkudata(L, 2, "gr.material");
  luaL_argcheck(L, matdata != 0, 2, "Material expected");

  Material * material = matdata->material;
  self->material.kd = material->kd;
  self->material.ks = material->ks;
  self->material.shininess = material->shininess;

  return 0;
}

// Set a physics node's material
// This will determine the color of the explosion produced when destroyed
extern "C"
int gr_physics_node_set_material_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  PhysicsNode* self = dynamic_cast<PhysicsNode*>(selfdata->node);

  luaL_argcheck(L, self != 0, 1, "Physics node expected");

  gr_material_ud* matdata = (gr_material_ud*)luaL_checkudata(L, 2, "gr.material");
  luaL_argcheck(L, matdata != 0, 2, "Material expected");

  Material * material = matdata->material;
  self->material.kd = material->kd;
  self->material.ks = material->ks;
  self->material.shininess = material->shininess;

  return 0;
}

// Set a animation node's material
// This will determine the color of the explosion produced when destroyed
extern "C"
int gr_animation_node_set_material_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  AnimationNode* self = dynamic_cast<AnimationNode*>(selfdata->node);

  luaL_argcheck(L, self != 0, 1, "Animation node expected");

  gr_material_ud* matdata = (gr_material_ud*)luaL_checkudata(L, 2, "gr.material");
  luaL_argcheck(L, matdata != 0, 2, "Material expected");

  Material * material = matdata->material;
  self->material.kd = material->kd;
  self->material.ks = material->ks;
  self->material.shininess = material->shininess;

  return 0;
}

// Add a scaling transformation to a node.
extern "C"
int gr_node_scale_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  SceneNode* self = selfdata->node;

  double values[3];

  for (int i = 0; i < 3; i++) {
    values[i] = luaL_checknumber(L, i + 2);
  }

    self->scale(glm::vec3(values[0], values[1], values[2]));

  return 0;
}

// Add a translation to a node.
extern "C"
int gr_node_translate_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  SceneNode* self = selfdata->node;

  double values[3];

  for (int i = 0; i < 3; i++) {
    values[i] = luaL_checknumber(L, i + 2);
  }

  self->translate(glm::vec3(values[0], values[1], values[2]));

  return 0;
}

// Rotate a node.
extern "C"
int gr_node_rotate_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  SceneNode* self = selfdata->node;

  const char* axis_string = luaL_checkstring(L, 2);

  luaL_argcheck(L, axis_string
                && std::strlen(axis_string) == 1, 2, "Single character expected");
  char axis = std::tolower(axis_string[0]);

  luaL_argcheck(L, axis >= 'x' && axis <= 'z', 2, "Axis must be x, y or z");

  double angle = luaL_checknumber(L, 3);

  self->rotate(axis, angle);

  return 0;
}

// Add a keyframe
extern "C"
int gr_animation_node_add_keyframe_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  AnimationNode* self = dynamic_cast<AnimationNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "AnimationNode node expected");

  unsigned int num_keyframes = luaL_checknumber(L, 2);

  for (unsigned int i=0; i<num_keyframes; i++) {
    self->addKeyframe();
  }

  return 0;
}

// Add a scaling transformation to a keyframe.
extern "C"
int gr_animation_node_scale_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  AnimationNode* self = dynamic_cast<AnimationNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "AnimationNode node expected");

  double values[3];

  for (int i = 0; i < 3; i++) {
    values[i] = luaL_checknumber(L, i + 2);
  }

  unsigned int keyframe = luaL_checknumber(L, 5);

  self->scaleKeyframe(glm::vec3(values[0], values[1], values[2]), keyframe);

  return 0;
}

// Add a translation to a keyframe.
extern "C"
int gr_animation_node_translate_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  AnimationNode* self = dynamic_cast<AnimationNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "AnimationNode node expected");

  double values[3];

  for (int i = 0; i < 3; i++) {
    values[i] = luaL_checknumber(L, i + 2);
  }

  unsigned int keyframe = luaL_checknumber(L, 5);

  self->translateKeyframe(glm::vec3(values[0], values[1], values[2]), keyframe);

  return 0;
}

// Add a rotation transformation to a keyframe.
extern "C"
int gr_animation_node_rotate_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  AnimationNode* self = dynamic_cast<AnimationNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "AnimationNode node expected");

  const char* axis_string = luaL_checkstring(L, 2);

  luaL_argcheck(L, axis_string
                && std::strlen(axis_string) == 1, 2, "Single character expected");
  char axis = std::tolower(axis_string[0]);

  luaL_argcheck(L, axis >= 'x' && axis <= 'z', 2, "Axis must be x, y or z");

  double angle = luaL_checknumber(L, 3);

  unsigned int keyframe = luaL_checknumber(L, 4);

  self->rotateKeyframe(axis, angle, keyframe);

  return 0;
}

// Set a physics node's velocity
extern "C"
int gr_physics_node_set_velocity_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  PhysicsNode* self = dynamic_cast<PhysicsNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "Physics node expected");

  double values[3];

  for (int i = 0; i < 3; i++) {
    values[i] = luaL_checknumber(L, i + 2);
  }

  self->set_velocity(glm::vec3(values[0], values[1], values[2]));

  return 0;
}

// Toggle gravity for a velocity node
extern "C"
int gr_node_set_gravity_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* selfdata = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, selfdata != 0, 1, "Node expected");

  PhysicsNode* self = dynamic_cast<PhysicsNode*>(selfdata->node);
  luaL_argcheck(L, self != 0, 1, "Physics node expected");

  bool g =  luaL_checknumber(L, 2);

  self->set_gravity(g);

  return 0;
}

// Garbage collection function for lua.
extern "C"
int gr_node_gc_cmd(lua_State* L)
{
  GRLUA_DEBUG_CALL;

  gr_node_ud* data = (gr_node_ud*)luaL_checkudata(L, 1, "gr.node");
  luaL_argcheck(L, data != 0, 1, "Node expected");

  // Note that we don't delete the node here. This is because we still
  // want the scene to be around when we close the lua interpreter,
  // but at that point everything will be garbage collected.
  //
  // If data->node happened to be a reference-counted pointer, this
  // will in fact just decrease lua's reference to it, so it's not a
  // bad thing to include this line.
  data->node = 0;

  return 0;
}

// This is where all the "global" functions in our library are
// declared.
// If you want to add a new non-member function, add it here.
static const luaL_Reg grlib_functions[] = {
  {"node", gr_node_cmd},
  {"joint", gr_joint_cmd},
  {"mesh", gr_mesh_cmd},
  {"material", gr_material_cmd},
  {"physics", gr_physics_cmd},
  {"animation", gr_animation_cmd},
  {0, 0}
};

// This is where all the member functions for "gr.node" objects are
// declared. Since all the other objects (e.g. materials) are so
// simple, we only really need to make member functions for nodes.
//
// If you want to add a new member function for gr.node, add it
// here.
//
// We could have used inheritance in lua to match the inheritance
// between different node types, but it's easier to just give all
// nodes the same Lua type and then do any additional type checking in
// the appropriate member functions (e.g. gr_node_set_material_cmd
// ensures that the node is a GeometryNode, see above).
static const luaL_Reg grlib_node_methods[] = {
  {"__gc", gr_node_gc_cmd},
  {"add_child", gr_node_add_child_cmd},
  {"set_material", gr_node_set_material_cmd},
  {"set_physics_material", gr_physics_node_set_material_cmd},
  {"set_animation_material", gr_animation_node_set_material_cmd},
  {"scale", gr_node_scale_cmd},
  {"rotate", gr_node_rotate_cmd},
  {"translate", gr_node_translate_cmd},
  {"keyframe_add", gr_animation_node_add_keyframe_cmd},
  {"keyframe_scale", gr_animation_node_scale_cmd},
  {"keyframe_rotate", gr_animation_node_rotate_cmd},
  {"keyframe_translate", gr_animation_node_translate_cmd},
  {"set_velocity", gr_physics_node_set_velocity_cmd},
  {"set_gravity", gr_node_set_gravity_cmd},
  {0, 0}
};

// This function calls the lua interpreter to do the actual importing
SceneNode* import_lua(const std::string& filename)
{
  GRLUA_DEBUG("Importing scene from " << filename);

  // Start a lua interpreter
  lua_State* L = luaL_newstate();

  GRLUA_DEBUG("Loading base libraries");

  // Load some base library
  luaL_openlibs(L);


  GRLUA_DEBUG("Setting up our functions");

  // Set up the metatable for gr.node
  luaL_newmetatable(L, "gr.node");
  lua_pushstring(L, "__index");
  lua_pushvalue(L, -2);
  lua_settable(L, -3);

  // Load the gr.node methods

  luaL_setfuncs(L, grlib_node_methods, 0);

  // Load the gr functions
  luaL_setfuncs(L, grlib_functions, 0);
  lua_setglobal(L, "gr");

  GRLUA_DEBUG("Parsing the scene");
  // Now parse the actual scene
  if (luaL_loadfile(L, filename.c_str()) || lua_pcall(L, 0, 1, 0)) {
    std::cerr << "Error loading " << filename << ": " << lua_tostring(L, -1) << std::endl;
    return 0;
  }

  GRLUA_DEBUG("Getting back the node");

  // Pull the returned node off the stack
  gr_node_ud* data = (gr_node_ud*)luaL_checkudata(L, -1, "gr.node");
  if (!data) {
    std::cerr << "Error loading " << filename << ": Must return the root node." << std::endl;
    return 0;
  }

  // Store it
  SceneNode* node = data->node;

  GRLUA_DEBUG("Closing the interpreter");

  // Close the interpreter, free up any resources not needed
  lua_close(L);

  // And return the node
  return node;
}

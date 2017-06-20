#include <glm/ext.hpp>

#include "A4.hpp"

using namespace std;
using namespace glm;

void traverseScene(SceneNode* root)
{
  // Do some action
  for (SceneNode* child: root->children) {
    traverseScene(child);
  }
}

void A4_Render(
    // What to render
    SceneNode * root,

    // Image to write to, set to a given width and height
    Image & image,

    // Viewing parameters
    const vec3 & eye,
    const vec3 & view,
    const vec3 & up,
    double fovy,

    // Lighting parameters
    const vec3 & ambient,
    const list<Light *> & lights
) {

  cout << "Calling A4_Render(\n" <<
      "\t" << *root <<
          "\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
          "\t" << "eye:  " << to_string(eye) << endl <<
      "\t" << "view: " << to_string(view) << endl <<
      "\t" << "up:   " << to_string(up) << endl <<
      "\t" << "fovy: " << fovy << endl <<
          "\t" << "ambient: " << to_string(ambient) << endl <<
      "\t" << "lights{" << endl;

  for(const Light * light : lights) {
    cout << "\t\t" <<  *light << endl;
  }
  cout << "\t}" << endl;
  cout <<")" << endl;

  size_t h = image.height();
  size_t w = image.width();

  mat4 T_1 = translate(mat4(), vec3(-float(w)/2, float(h)/2, 1.0f));

  float t = 2*tan(fovy/2);
  float s = float(w)/float(h) * t;
  mat4 S_2 = scale(mat4(), vec3(-t/h, t/h, 1.0f));

  vec3 view_n = normalize(view);
  vec3 u = normalize(cross(up, view_n));
  vec3 v = cross(view_n, u);
  mat4 R_3(
    vec4( u, 0.0f ),
    vec4( v, 0.0f ),
    vec4( view_n, 0.0f ),
    vec4( 0.0f, 0.0f, 0.0f, 1.0f )
  );

  mat4 T_4 = translate(mat4(), vec3(eye.x, eye.y, eye.z));

  for (uint y = 0; y < h; ++y) {
    for (uint x = 0; x < w; ++x) {
      vec4 point(x, y, 0.0f, 1.0f);
      point = T_1 * point;

      // Red: increasing from top to bottom
      image(x, y, 0) = (double)y / h;
      // Green: increasing from left to right
      image(x, y, 1) = (double)x / w;
      // Blue: in lower-left and upper-right corners
      image(x, y, 2) = ((y < h/2 && x < w/2)
              || (y >= h/2 && x >= w/2)) ? 1.0 : 0.0;
    }
  }

}

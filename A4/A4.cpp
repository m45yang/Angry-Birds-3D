#include <glm/ext.hpp>

#include "A4.hpp"

using namespace std;
using namespace glm;


bool intersect(Ray r, float *t, vec3 *N, vec3 uv, vec3 *kd, vec3 *ks, vec3 *ke, const SceneNode & node)
{
  bool isIntersect = false;
  // cerr << node.m_name << endl;
  const GeometryNode * geometryNode = dynamic_cast<const GeometryNode *>(&node);

  if (geometryNode) {
    Primitive * primitive = geometryNode->m_primitive;
    Material * material = geometryNode->m_material;
    PhongMaterial * phongMaterial = dynamic_cast<PhongMaterial *>(material);

    // Do sphere intersection if object is a sphere
    NonhierSphere * nonhierSphere = dynamic_cast<NonhierSphere*>(primitive);
    if (nonhierSphere) {
      vec3 position = nonhierSphere->getPosition();
      double radius = nonhierSphere->getRadius();

      double A = dot(uv-r.origin, uv-r.origin);
      double B = 2*dot(uv-r.origin, r.origin-position);
      double C = dot(r.origin-position, r.origin-position) - radius*radius;
      double roots[2];
      size_t numRoots = quadraticRoots(A, B, C, roots);

      if (numRoots == 0) {
        // No intersection
      }
      else if (numRoots == 1) {
        // Ray tangent to sphere
      }
      else if (numRoots == 2) {
        // Ray intersects sphere, test for closer point
        isIntersect = true;
        double new_t = roots[0] ? roots[0] < roots[1] : roots[1];
        if (*t == -1.0f || new_t < *t) {
          *t = new_t;
          *N = r.origin + (*t)*r.direction - position;
          *kd = phongMaterial->getKd();
          *ks = phongMaterial->getKs();
        }
      }
    }

    // Do box intersection if object is a box
    NonhierBox * nonhierBox = dynamic_cast<NonhierBox*>(primitive);
    if (nonhierBox) {

    }
  }

  for (SceneNode* child : node.children) {
    isIntersect = intersect(r, t, N, uv, kd, ks, ke, *child) || isIntersect;
  }

  return isIntersect;
}

vec3 directLight(vec3 p, vec3 N, vec3 uv, const SceneNode & root, const list<Light *> & lights)
{
  vec3 kd, ks, ke, col;
  vec3 light(0, 0, 0);
  float t;
  list<Light*> unobstructedLights;
  list<Light *>::const_iterator it1;
  list<Light *>::iterator it2;

  for (it1=lights.begin(); it1!=lights.end(); it1++) {
    t = -1.0f;
    Ray r = Ray(p, (*it1)->position-p);
    if (!intersect(r, &t, &N, uv, &kd, &ks, &ke, root)) {
      unobstructedLights.push_back(*it1);
    }
  }

  for (it2=unobstructedLights.begin(); it2!=unobstructedLights.end(); it2++) {
    light[0] += (*it2)->colour[0] / ((*it2)->falloff[0] + (*it2)->falloff[1] + (*it2)->falloff[2]);
    light[1] += (*it2)->colour[1] / ((*it2)->falloff[0] + (*it2)->falloff[1] + (*it2)->falloff[2]);
    light[2] += (*it2)->colour[2] / ((*it2)->falloff[0] + (*it2)->falloff[1] + (*it2)->falloff[2]);
  }

  return light;
}

vec3 rayColor(Ray r, vec3 uv, int hits, const SceneNode & root, const list<Light *> & lights)
{
  vec3 kd, ks, ke, col, N, p;
  float t = -1.0f;

  if (intersect(r, &t, &N, uv, &kd, &ks, &ke, root)) {
    col = kd;
    p = uv + t*r.direction;
    if (kd != vec3(0,0,0)) {
      // Compute direct light
      vec3 light = directLight(p, N, uv, root, lights);
      col[0] = kd[0] * light[0];
      col[1] = kd[1] * light[1];
      col[2] = kd[2] * light[2];
    }
  }
  else {
    return vec3(0.0f, 0.0f, 0.0f);
  }

  return col;
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

  mat4 T_1 = translate(mat4(), vec3(-float(w)/2, -float(h)/2, 1.0f));

  float t = 2*tan(radians(fovy/2));
  float s = float(w)/float(h) * t;
  mat4 S_2 = scale(mat4(), vec3(-s/w, t/h, 1.0f));

  vec3 view_n = normalize(view);
  vec3 u = normalize(cross(up, view_n));
  vec3 v = cross(view_n, u);
  mat4 R_3(
    vec4( u, 0.0f ),
    vec4( v, 0.0f ),
    vec4( view_n, 0.0f ),
    vec4( 0.0f, 0.0f, 0.0f, 1.0f )
  );

  mat4 T_4 = translate(mat4(), eye);

  for (uint y = 0; y < h; ++y) {
    for (uint x = 0; x < w; ++x) {
      // Transform point to world space
      vec4 point(x, y, 0.0f, 1.0f);
      point = T_4 * R_3 * S_2 * T_1 * point;
      vec3 direction = vec3(point.x, point.y, point.z) - eye;

      // Send a ray through each pixel
      Ray ray = Ray(eye, direction);

      // Get the color for the ray
      vec3 color = rayColor(ray, vec3(point.x, point.y, point.z), 0, *root, lights);

      // Red
      image(x, y, 0) = color.x;
      // Green
      image(x, y, 1) = color.y;
      // Blue
      image(x, y, 2) = color.z;
    }
  }

}

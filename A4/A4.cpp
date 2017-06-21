#include <glm/ext.hpp>

#include "A4.hpp"

using namespace std;
using namespace glm;

bool triangleIntersect(Ray ray, vec3 p0, vec3 p1, vec3 p2, vec3 uv)
{
  vec3 x, y, z, r;
  mat3 d, d0, d1, d2;
  double det, det0, det1, det2;
  double beta, gamma;

  x = vec3(p1.x-p0.x, p2.x-p0.x, uv.x-ray.origin.x);
  y = vec3(p1.y-p0.y, p2.y-p0.y, uv.y-ray.origin.y);
  z = vec3(p1.z-p0.z, p2.z-p0.z, uv.z-ray.origin.z);
  r = vec3(ray.origin.x-p0.x, ray.origin.y-p0.y, ray.origin.z-p0.z);

  d = mat3(
    vec3(x[0], y[0], z[0]),
    vec3(x[1], y[1], z[1]),
    vec3(x[2], y[2], z[2])
  );
  det = determinant(d);
  d0 = mat3(
    r,
    vec3(x[1], y[1], z[1]),
    vec3(x[2], y[2], z[2])
  );
  det0 = determinant(d0);
  d1 = mat3(
    vec3(x[0], y[0], z[0]),
    r,
    vec3(x[2], y[2], z[2])
  );
  det1 = determinant(d1);
  d2 = mat3(
    vec3(x[0], y[0], z[0]),
    vec3(x[1], y[1], z[1]),
    r
  );
  det2 = determinant(d2);

  beta = det0/det;
  gamma = det1/det;

  return (beta >= 0 && gamma >= 0 && beta + gamma <= 1);
}

bool intersect(
  Ray ray,
  double *t,
  vec3 *N,
  vec3 uv,
  vec3 *kd,
  vec3 *ks,
  vec3 *ke,
  double *shine,
  const SceneNode & node
)
{
  bool isIntersect = false;
  double new_t;
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
      double A = dot(uv-ray.origin, uv-ray.origin);
      double B = dot(uv-ray.origin, ray.origin-position) * 2;
      double C = dot(ray.origin-position, ray.origin-position) - radius*radius;
      double roots[2];
      size_t numRoots = quadraticRoots(A, B, C, roots);

      if (numRoots == 2) {
        // Ray intersects sphere, test for closer point
        new_t = roots[0] < roots[1] ? roots[0] : roots[1];
        isIntersect = true;
        if (*t == -1.0 || new_t < *t) {
          *t = new_t;
          *N = normalize(ray.origin + (*t)*ray.direction - position);
          *kd = phongMaterial->getKd();
          *ks = phongMaterial->getKs();
          *shine = phongMaterial->getShininess();
          *ke = (*kd)/4;
        }
      }
    }

    // Do box intersection if object is a box
    NonhierBox * nonhierBox = dynamic_cast<NonhierBox*>(primitive);
    if (nonhierBox) {

    }

    // Do box intersection if object is a mesh
    Mesh * mesh = dynamic_cast<Mesh*>(primitive);
    if (mesh) {
      vector<Triangle>::iterator it;
      vec3 p0, p1, p2, x, y, z, r;
      mat3 d, d0, d1, d2;
      double det, det0, det1, det2;
      double beta, gamma;

      for(it=mesh->m_faces.begin(); it!=mesh->m_faces.end(); it++) {
        p0 = mesh->m_vertices[it->v1];
        p1 = mesh->m_vertices[it->v2];
        p2 = mesh->m_vertices[it->v3];

        if (triangleIntersect(ray, p0, p1, p2, uv)) {
          isIntersect = true;
          new_t = det2/det;
          if (*t == -1.0 || new_t < *t) {
            *N = normalize(triangleNormal(p0, p1, p2));
            *kd = phongMaterial->getKd();
            *ks = phongMaterial->getKs();
            *shine = phongMaterial->getShininess();
            *ke = (*kd)/4;
          }
        }
      }
    }
  }

  for (SceneNode* child : node.children) {
    bool result = intersect(ray, t, N, uv, kd, ks, ke, shine, *child);
    isIntersect = isIntersect || result;
  }

  return isIntersect;
}

vec3 directLight(
  vec3 p,
  vec3 N,
  vec3 uv,
  vec3 v,
  vec3 kd,
  vec3 ks,
  double shine,
  const SceneNode & root,
  const list<Light *> & lights
)
{
  vec3 ke, col, reflected, light;
  vec3 combinedLights(0, 0, 0);
  double t, r;
  list<Light*> unobstructedLights;
  list<Light *>::const_iterator it;

  for (it=lights.begin(); it!=lights.end(); it++) {
    t = -1.0;
    Ray ray = Ray((*it)->position, p - (*it)->position);

    if (!intersect(ray, &t, &N, uv, &kd, &ks, &ke, &shine, root)) {
      light = (*it)->position - p;
      r = length(light);
      light = normalize(light);
      reflected = normalize(-light + 2*dot(light, N)*N);
      vec3 specular = kd + ks*(pow(dot(reflected, v), shine))/(dot(N,light));
      combinedLights[0] += specular[0] * (*it)->colour[0] * dot(light, N) / ((*it)->falloff[0] + (*it)->falloff[1]*r + (*it)->falloff[2]*pow(r,2));
      combinedLights[1] += specular[1] * (*it)->colour[1] * dot(light, N) / ((*it)->falloff[0] + (*it)->falloff[1]*r + (*it)->falloff[2]*pow(r,2));
      combinedLights[2] += specular[2] * (*it)->colour[2] * dot(light, N) / ((*it)->falloff[0] + (*it)->falloff[1]*r + (*it)->falloff[2]*pow(r,2));
    }
  }

  return combinedLights;
}

vec3 rayColor(Ray r, vec3 uv, vec3 ambient, int hits, const SceneNode & root, const list<Light *> & lights)
{
  vec3 kd, ks, ke, col, N, p;
  double t = -1.0;
  double shine = 1.0;

  if (intersect(r, &t, &N, uv, &kd, &ks, &ke, &shine, root)) {
    col = kd + ke*ambient;
    p = r.origin + t*r.direction;
    if (kd != vec3(0,0,0)) {
      // Compute direct light
      vec3 light = directLight(p, N, uv, normalize(r.origin-p), kd, ks, shine, root, lights);
      col[0] = kd[0]*(ke[0]*ambient[0] + light[0]);
      col[1] = kd[1]*(ke[1]*ambient[1] + light[1]);
      col[2] = kd[2]*(ke[2]*ambient[2] + light[2]);
    }
  }
  else {
    return vec3(0.0, 0.0, 0.0);
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

  mat4 T_1 = translate(mat4(), vec3(-double(w)/2, -double(h)/2, 1.0));

  double t = 2*tan(radians(fovy/2));
  double s = double(w)/double(h) * t;
  mat4 S_2 = scale(mat4(), vec3(-s/w, t/h, 1.0));

  vec3 view_n = normalize(view);
  vec3 u = normalize(cross(up, view_n));
  vec3 v = cross(view_n, u);
  mat4 R_3(
    vec4( u, 0.0 ),
    vec4( v, 0.0 ),
    vec4( view_n, 0.0 ),
    vec4( 0.0, 0.0, 0.0, 1.0 )
  );

  mat4 T_4 = translate(mat4(), eye);

  for (uint y = 0; y < h; ++y) {
    for (uint x = 0; x < w; ++x) {
      // Transform point to world space
      vec4 point(x, y, 0.0, 1.0);
      point = T_4 * R_3 * S_2 * T_1 * point;
      vec3 direction = vec3(point.x, point.y, point.z) - eye;

      // Send a ray through each pixel
      Ray ray = Ray(eye, direction);

      // Get the color for the ray
      vec3 color = rayColor(ray, vec3(point.x, point.y, point.z), ambient, 0, *root, lights);

      // Red
      image(x, y, 0) = color.x;
      // Green
      image(x, y, 1) = color.y;
      // Blue
      image(x, y, 2) = color.z;
    }
  }

}

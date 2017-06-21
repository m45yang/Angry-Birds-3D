#pragma once

#include <glm/glm.hpp>

#include "GeometryNode.hpp"
#include "SceneNode.hpp"
#include "Light.hpp"
#include "Image.hpp"
#include "PhongMaterial.hpp"
#include "Ray.hpp"
#include "polyroots.hpp"

bool intersect(Ray r, float *t, glm::vec3 *N, glm::vec3 uv, glm::vec3 *kd, glm::vec3 *ks, glm::vec3 *ke, const SceneNode & node);

glm::vec3 directLight(glm::vec3 p, glm::vec3 N, glm::vec3 uv, const SceneNode & root, const std::list<Light *> & lights);

glm::vec3 rayColor(Ray r, glm::vec3 uv, int hits, const SceneNode & root, const std::list<Light *> & lights);

void A4_Render(
    // What to render
    SceneNode * root,

    // Image to write to, set to a given width and height
    Image & image,

    // Viewing parameters
    const glm::vec3 & eye,
    const glm::vec3 & view,
    const glm::vec3 & up,
    double fovy,

    // Lighting parameters
    const glm::vec3 & ambient,
    const std::list<Light *> & lights
);

#include "Vec3.h"
#include "Sampler.h"
#include "Materials/Material.h"
#include "Ray.h"
#include "BBox.h"
#include "Sphere.h"
#include "Face.h"
#include "Intersect.h"
#include "Accelerator/KdTree.h"
#include "ModelSet.h"
#include "Scene.h"

using namespace hiraishi;

void Scene::init(const int w, const int h) {
    model.readMtl(mtlPath.c_str());
    model.readObj(objPath.c_str());
    model.makeFaceEquations();
    model.initKdTree();
    model.initVColor();
    // model.initLightArea(); // currently cannot use
}

Intersect Scene::intersect(const Ray& ray, Random& rng) const {
    return model.intersect(ray);
}

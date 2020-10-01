#include <assert.h>
#include "Constant.h"
#include "Vec3.h"
#include "Materials/Material.h"
#include "Ray.h"
#include "Sphere.h"

using namespace hiraishi;

void Sphere::print() {
    printf("radius = %lf, center = (%lf, %lf, %lf)\n", radius, center.x, center.y, center.z);
}

bool Sphere::nearlyEqual(const Sphere& s) {
    if (H_EPSILON < fabs(s.getRadius() - radius)) return false;
    if (!center.nearlyEqual(s.getCenter())) return false;
    return true;
}

void Sphere::TEST_grow() {
    radius = 1.0;
    center = Vec3(0.0, 0.0, 0.0);
    grow(Vec3(2.0, 0.0, 0.0));
    assert(nearlyEqual(Sphere(Vec3(0.5, 0.0, 0.0), 1.5)));
}

void Sphere::grow(const Vec3& v) {
    const double dist = Vec3::dist(center, v);
    if (dist < radius) return;
    Vec3 v2c = center - v;
    v2c = v2c.normalize();
    radius = (dist + radius) * 0.5;
    center = v + v2c * radius;
}

bool Sphere::intersect(const Ray& ray, double* t) const {
    assert(fabs(ray.d.length() - 1.0) < H_EPSILON);
    const double b = Vec3::dot(ray.d, ray.o - center);
    const double c = Vec3::dist2(ray.o, center) - radius * radius;
    const double D = b * b - c;
    if (D < 0.0) return false;
    const double tPlus = -b + sqrt(D);
    if (tPlus < 0.0) return false;
    const double tMinus = -b - sqrt(D);
    if (tMinus < 0.0) {
        *t = tPlus;
    } else {
        *t = tMinus;
    }
    return true;
}
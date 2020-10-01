#include <vector>
#include <assert.h>
#include "Constant.h"
#include "Vec3.h"
#include "Materials/Material.h"
#include "Ray.h"
#include "BBox.h"
#include "Face.h"

using namespace hiraishi;

Face::Face(const std::vector<int>& vi) {
    vIndices.clear();
    for (std::vector<int>::const_iterator i = vi.begin(); i != vi.end(); i++) {
        vIndices.push_back(*i);
    }
}

void Face::makeEquation(const Vec3& p0, const Vec3& p1, const Vec3& p2) {
    pos = p0;
    Vec3 answer = Vec3::cross(p1 - p0, p2 - p0);
    normal = answer.normalize();
    midPos = (p0 + p1 + p2) / 3.0;
    bbox.min = Vec3(fmin(fmin(p0.x, p1.x), p2.x),
                    fmin(fmin(p0.y, p1.y), p2.y),
                    fmin(fmin(p0.z, p1.z), p2.z));
    bbox.max = Vec3(fmax(fmax(p0.x, p1.x), p2.x),
                    fmax(fmax(p0.y, p1.y), p2.y),
                    fmax(fmax(p0.z, p1.z), p2.z));
}

bool Face::intersect(const Vec3* vert, const Ray& ray, double* tParam, Vec3& isectPos, Vec3& isectNormal, Material** mtlPtr) const {
    const Vec3 o = ray.o;
    const Vec3 d = ray.d;
    const Vec3 n = getNormal();
    //“§‰ß‚µ‚Ä‚¢‚È‚¢ê‡‚Ì‚Ý— ‘¤‚©‚ç‚ÌŒð·‚ð’e‚­
    if (this->mtlPtr->illum != 7 && this->mtlPtr->illum != 10 && this->mtlPtr->illum != 11) {
        const double dot = Vec3::dot(n, -d);
        if (dot < H_EPSILON) return false;
    }

    const Vec3 e1 = vert[1] - vert[0];
    const Vec3 e2 = vert[2] - vert[0];
    const Vec3 alpha = Vec3::cross(d, e2);
    const double det = Vec3::dot(e1, alpha);
    if (-H_EPSILON < det && det < H_EPSILON) return false;

    const double invDet = 1.0 / det;
    const Vec3 r = o - vert[0];
    const double u = Vec3::dot(alpha, r) * invDet;
    if (u < 0.0 || 1.0 < u) return false;

    const Vec3 beta = Vec3::cross(r, e1);
    const double v = Vec3::dot(d, beta) * invDet;
    if (v < 0.0 || 1.0 < u + v) return false;

    const double t = Vec3::dot(e2, beta) * invDet;
    if (t < H_EPSILON) return false;

    *tParam = t;
    isectPos = o + d * t;
    isectNormal = n;
    *mtlPtr = this->mtlPtr;
    return true;
}

void Face::print() const {
    printf("f");
    for (std::vector<long>::const_iterator v = vIndices.begin(); v != vIndices.end(); v++) {
        printf(" %d", *v);
    }
    printf("\n");
}
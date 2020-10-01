#include "Vec3.h"
#include "Materials/Material.h"
#include "Ray.h"

using namespace hiraishi;

Vec3 Ray::localIllumination(const Material* hitMtl, const Vec3& isectPos, const Vec3& isectNormal) const {
    const Vec3 lightPos(-0.005, 1.58, -0.03);
    Vec3 lightVec = lightPos - isectPos;
    const double distance = lightVec.length();
    lightVec = lightVec.normalize();
    const Vec3 kd = hitMtl->Kd;
    const double diffuse = Vec3::dot(lightVec, isectNormal);
    if (hitMtl->name == "light") {
        return hitMtl->Ke;
    }
    return kd / M_PI * Vec3(1.5, 1.5, 1.5) * std::fmin(std::fmax(0.0, diffuse), 1.0);
}
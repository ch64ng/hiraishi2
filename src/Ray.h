#pragma once

namespace hiraishi {
    struct Ray {
        Ray() {}
        Ray(const Vec3& origin, const Vec3& direction)
            : o(origin), d(direction) {
            d = d.normalize();
        }
        ~Ray() {}

        Vec3 o;
        Vec3 d;

        Vec3 localIllumination(const Material* hitMtl, const Vec3& isectPos, const Vec3& isectNormal) const;
    };
}
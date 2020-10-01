#pragma once

#include "Vec3.h"

namespace hiraishi {
    namespace Trig {
        inline double cosTheta(const Vec3& v) {
            return v.y;
        }

        inline double cos2Theta(const Vec3& v) {
            return v.y * v.y;
        }

        inline double sin2Theta(const Vec3& v) {
            return fmax(0.0, 1.0 - cos2Theta(v));
        }

        inline double sinTheta(const Vec3& v) {
            return sqrt(sin2Theta(v));
        }

        inline double tanTheta(const Vec3& v) {
            return sinTheta(v) / cosTheta(v);
        }

        inline double tan2Theta(const Vec3& v) {
            return sin2Theta(v) / cos2Theta(v);
        }

        inline double cosPhi(const Vec3& v) {
            const double st = sinTheta(v);
            return (st == 0) ? 1.0 : fmin(fmax(v.x / st, -1.0), 1.0);
        }

        inline double sinPhi(const Vec3& v) {
            const double st = sinTheta(v);
            return (st == 0) ? 1.0 : fmin(fmax(v.z / st, -1.0), 1.0);
        }

        inline double cos2Phi(const Vec3& v) {
            const double cp = cosPhi(v);
            return cp * cp;
        }

        inline double sin2Phi(const Vec3& v) {
            const double sp = sinPhi(v);
            return sp * sp;
        }
    };
}
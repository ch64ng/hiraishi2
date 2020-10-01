#include "Vec3.h"
#include "Sampler.h"

using namespace hiraishi;

Vec3 Sampler::uniformSampleTriangle(const Vec3* v, double r1, double r2) {
    if (1.0 <= r1 + r2) {
        r1 = 1.0 - r1;
        r2 = 1.0 - r2;
    }
    return v[0] + (v[1] - v[0]) * r1 + (v[2] - v[0]) * r2;
}

Vec3 Sampler::uniformSampleSquare2D(double r1, double r2) const {
    return Vec3(r1, r2, 0.0);
}

Vec3 Sampler::R2Sampler() {
    // http://extremelearning.com.au/unreasonable-effectiveness-of-quasirandom-sequences/
    const double x = fmod((0.5 + a1 * (double)index), 1.0);
    const double y = fmod((0.5 + a2 * (double)index), 1.0);
    index++;
    return Vec3(x, y, 0.0);
}

Vec3 Sampler::GapSampler() {
    double x = 0.0;
    double y = 0.0;
    if (index % 2 == 1) {
        x = 1.0;
        y = 1.0;
    }
    index++;
    return Vec3(x, y, 0.0);
}
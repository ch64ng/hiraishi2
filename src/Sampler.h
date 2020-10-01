#pragma once

namespace hiraishi {
    class Sampler {
    private:
        const double g = 1.32471795724474602596090885447809;
        const double a1 = 0.7548776662466927; // a1 = 1.0 / g;
        const double a2 = 0.5698402909980532; // a2 = 1.0 / (g * g);

        int index = 0;

    public:
        Sampler() {}
        ~Sampler() {}

        static Vec3 uniformSampleTriangle(const Vec3* v, double r1, double r2);
        Vec3 uniformSampleSquare2D(double r1, double r2) const;
        Vec3 R2Sampler();
        Vec3 GapSampler();
    };
}
#pragma once

namespace hiraishi {
    class BSDF {
    private:
        Ray ray;
        Intersect isect;
        double cosTerm;
        double ax, ay;

        Vec3 randomReflectDir(Random& rng) const;
        Vec3 mirrorReflectDir(const Vec3& n, const Vec3& dir) const;
        Vec3 refractDir(const Vec3& n, const Vec3& dir, const double ior) const;
        Vec3 fresnelDir(const Vec3& n, const Vec3& dir, const double n1, const double n2, const double p) const;
        double fresnelSchlickApprox(const double ior, const double cosd) const;
        Vec3 DisneyBRDF(double& pdf) const;
        Vec3 evaluateF(const Vec3& w, const Vec3& wm) const;
        double smithGGXG1(const Vec3& w) const;
        double evaluateG(const Vec3& wo, const Vec3& wi, const Vec3& wm) const;
        double evaluateD(const Vec3& wm) const;
        Vec3 sampleMicrofacetNormal(Random& rng);

    public:
        BSDF() {}
        BSDF(const Ray& ray, const Intersect& isect, const double cosTerm) : ray(ray), isect(isect), cosTerm(cosTerm) {}
        ~BSDF() {}

        static Vec3 randomDirSphere(Random& rng);
        Vec3 evaluateDirection(Random& rng);
        Vec3 evaluateBSDF(double& pdf) const;
        Vec3 evaluateSpectrumDirection(Random& rng);
        Spectrum evaluateSpectrumBSDF(double& pdf, const Spectrum& kd) const;
    };
}
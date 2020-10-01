#pragma once

namespace hiraishi {
    class Camera {
    private:
        double fov    = 0.0; // radian
        double aspect = 0.0;

        Vec3 eye    = Vec3(0.0, 0.0, 0.0);
        Vec3 center = Vec3(0.0, 0.0, 0.0);
        Vec3 up     = Vec3(0.0, 1.0, 0.0);

        // Basis Vectors
        Vec3 w = Vec3(0.0, 0.0, 0.0);
        Vec3 u = Vec3(0.0, 0.0, 0.0);
        Vec3 v = Vec3(0.0, 0.0, 0.0);

    public:
        Camera() {}
        ~Camera() {}

        void setFovRad(const double _fovRad) { fov = _fovRad; }
        void setFovDeg(const double _fovDeg) { fov = _fovDeg * M_PI / 180.0; }
        void setAspect(const double _aspect) { aspect = _aspect; }
        void setEye(const Vec3& _eye) { eye = _eye; }
        void setCenter(const Vec3& _center) { center = _center; }
        const double& getFov() const { return fov; }
        const double& getAspect() const { return aspect; }
        const Vec3& getEye() const { return eye; }
        const Vec3& getCenter() const { return center; }

        void init(const int width, const int height);
        Vec3 samplePixel(const int i, const int j, const int width, const int height, Random& rng, Sampler& sampler) const;
    };
}
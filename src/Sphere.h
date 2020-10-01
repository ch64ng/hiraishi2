#pragma once

namespace hiraishi {
    class Sphere {
    private:
        Vec3 center;
        double radius;

    public:
        Sphere() {}
        Sphere(const Vec3 v, const double r) : center(v), radius(r) {}
        ~Sphere() {}

        Vec3 getCenter() const { return center; }
        double getRadius() const { return radius; }
        void setCenter(const Vec3 c) { center = c; }
        void setRadius(const double r) { radius = r; }
        void print();
        void TEST_grow();
        bool nearlyEqual(const Sphere& s);
        void grow(const Vec3& v);
        bool intersect(const Ray& ray, double* t) const;
    };
}
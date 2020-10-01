#include "Vec3.h"
#include "Sampler.h"
#include "Camera.h"

using namespace hiraishi;

void Camera::init(const int width, const int height) {
    aspect = (double)width / (double)height;
    w = (eye - center).normalize();
    u = Vec3::cross(up, w).normalize();
    v = Vec3::cross(w, u);
}

Vec3 Camera::samplePixel(const int i, const int j, const int width, const int height, Random& rng, Sampler& sampler) const {
    const double halfH = std::tan(fov * 0.5);
    const double halfW = aspect * halfH;
    const Vec3 screen_u = u * halfW * 2.0;
    const Vec3 screen_v = v * halfH * 2.0;
    const Vec3 screen_w = eye - u * halfW - v * halfH - w;
    const Vec3 rp = sampler.R2Sampler();
    const double x = ((double)i + rp.x) / width;
    const double y = ((double)j + rp.y) / height;
    const Vec3 target = screen_w + screen_u * x + screen_v * y;
    return target;
}
#pragma once

#include "Vec3.h"
#include "Ray.h"

namespace hiraishi {
    struct BBox {
        Vec3 min;
        Vec3 max;

        void grow(const BBox& bbox) {
            min = Vec3(fmin(min.x, bbox.min.x),
                       fmin(min.y, bbox.min.y),
                       fmin(min.z, bbox.min.z));
            max = Vec3(fmax(max.x, bbox.max.x),
                       fmax(max.y, bbox.max.y),
                       fmax(max.z, bbox.max.z));
        }

        bool intersect(const Ray& ray) {
            double tmin = (min.x - ray.o.x) / ray.d.x;
            double tmax = (max.x - ray.o.x) / ray.d.x;

            if (tmin > tmax) std::swap(tmin, tmax);

            double tymin = (min.y - ray.o.y) / ray.d.y;
            double tymax = (max.y - ray.o.y) / ray.d.y;

            if (tymin > tymax) std::swap(tymin, tymax);

            if ((tmin > tymax) || (tymin > tmax))
                return false;

            if (tymin > tmin)
                tmin = tymin;

            if (tymax < tmax)
                tmax = tymax;

            double tzmin = (min.z - ray.o.z) / ray.d.z;
            double tzmax = (max.z - ray.o.z) / ray.d.z;

            if (tzmin > tzmax) std::swap(tzmin, tzmax);

            if ((tmin > tzmax) || (tzmin > tmax))
                return false;

            if (tzmin > tmin)
                tmin = tzmin;

            if (tzmax < tmax)
                tmax = tzmax;

            return true;
        }
    };
}
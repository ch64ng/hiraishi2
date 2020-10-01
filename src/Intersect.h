#pragma once

#include "Constant.h"
#include "Vec3.h"
#include "Materials/Material.h"
#include "Intersect.h"

namespace hiraishi {
    struct Intersect {
        Intersect() {}
        ~Intersect() {}

        const Material* mtlPtr = NULL;
        double t = H_INFINITE;
        Vec3 pos;
        Vec3 normal;
        Vec3 wm;
        Vec3 wi;
    };
}
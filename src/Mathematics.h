#pragma once

namespace hiraishi {
    struct H_Math {
        inline static double lerp(double t, double v0, double v1) {
            return v0 * (1.0 - t) + v1 * t;
        }

        inline static double clamp(double val, double low, double high) {
            if (val < low) return low;
            else if (high < val) return high;
            else return val;
        }

        inline static double map(double val, double min0, double max0, double min1, double max1) {
            // map value from (min0, max0) to (min1, max1)
            return min1 + (val - min0) * (max1 - min1) / (max0 - min0);
        }
    };
}
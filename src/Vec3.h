#pragma once
#define _USE_MATH_DEFINES

#include <stdio.h>
#include <math.h>
#include "Constant.h"
#include "Random.h"
#include "Mathematics.h"

namespace hiraishi {
    struct Vec3 {
        double x, y, z;

        Vec3() : x(0), y(0), z(0) {}
        Vec3(const Vec3& v) : x(v.x), y(v.y), z(v.z) {}
        Vec3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
        Vec3(double d) : x(d), y(d), z(d) {}
        ~Vec3() {}

        //scalar operator
        inline Vec3 operator*(const double& d) const {
            Vec3 answer;
            answer.x = x * d;
            answer.y = y * d;
            answer.z = z * d;
            return answer;
        }

        inline Vec3 operator/(const double& d) const {
            Vec3 answer;
            answer.x = x / d;
            answer.y = y / d;
            answer.z = z / d;
            return answer;
        }

        //vector operator
        inline Vec3& operator=(const Vec3& v) {
            x = v.x;
            y = v.y;
            z = v.z;
            return *this;
        }

        inline Vec3 operator+(const Vec3& v) const {
            Vec3 answer;
            answer.x = x + v.x;
            answer.y = y + v.y;
            answer.z = z + v.z;
            return answer;
        }

        inline Vec3 operator-(const Vec3& v) const {
            Vec3 answer;
            answer.x = x - v.x;
            answer.y = y - v.y;
            answer.z = z - v.z;
            return answer;
        }

        inline Vec3 operator*(const Vec3& v) const {
            Vec3 answer;
            answer.x = x * v.x;
            answer.y = y * v.y;
            answer.z = z * v.z;
            return answer;
        }

        inline Vec3 operator/(const Vec3& v) const {
            Vec3 answer;
            answer.x = x / v.x;
            answer.y = y / v.y;
            answer.z = z / v.z;
            return answer;
        }

        inline Vec3 operator-() const {
            return Vec3(-x, -y, -z);
        }

        inline bool operator==(const Vec3& v) const {
            return x == v.x && y == v.y && z == v.z;
        }

        inline bool operator!=(const Vec3& v) const {
            return !(*this == v);
        }

        inline static double dot(const Vec3& v0, const Vec3& v1) {
            double answer;
            answer = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
            return answer;
        }

        inline static double absDot(const Vec3& v0, const Vec3& v1) {
            double answer;
            answer = v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
            return abs(answer);
        }

        inline static Vec3 cross(const Vec3& v0, const Vec3& v1) {
            Vec3 answer;
            answer.x = v0.y * v1.z - v0.z * v1.y;
            answer.y = v0.z * v1.x - v0.x * v1.z;
            answer.z = v0.x * v1.y - v0.y * v1.x;
            return answer;
        }

        inline double length() const {
            return sqrt(x * x + y * y + z * z);
        }

        inline double length2() const {
            return x * x + y * y + z * z;
        }

        inline Vec3 normalize() const {
            Vec3 answer;
            double length = this->length();
            answer.x = x / length;
            answer.y = y / length;
            answer.z = z / length;
            return answer;
        }

        inline static double dist(const Vec3& v0, const Vec3& v1) {
            double answer;
            answer = Vec3(v0 - v1).length();
            return answer;
        }

        inline static double dist2(const Vec3& v0, const Vec3& v1) {
            double answer;
            answer = Vec3(v0 - v1).length2();
            return answer;
        }

        inline void set(double _x, double _y, double _z) {
            x = _x;
            y = _y;
            z = _z;
        }

        inline void set(Vec3 v) {
            x = v.x;
            y = v.y;
            z = v.z;
        }

        inline static Vec3 zero() {
            return Vec3(0.0, 0.0, 0.0);
        }

        inline static Vec3 black() {
            return Vec3(0.0, 0.0, 0.0);
        }

        inline void print() {
            printf("(%.5lf, %.5lf, %.5lf)\n", x, y, z);
        }

        inline static Vec3 min(Vec3 a, Vec3 b) {
            Vec3 answer;
            answer.x = fmin(a.x, b.x);
            answer.y = fmin(a.y, b.y);
            answer.z = fmin(a.z, b.z);
            return answer;
        }

        inline static Vec3 max(Vec3 a, Vec3 b) {
            Vec3 answer;
            answer.x = fmax(a.x, b.x);
            answer.y = fmax(a.y, b.y);
            answer.z = fmax(a.z, b.z);
            return answer;
        }

        inline static Vec3 clamp(Vec3 val, double low = 0.0, double high = H_INFINITE) {
            Vec3 answer;
            answer.x = H_Math::clamp(val.x, low, high);
            answer.y = H_Math::clamp(val.y, low, high);
            answer.z = H_Math::clamp(val.z, low, high);
            return answer;
        }

        inline static Vec3 rotateX(const Vec3& v, const double& theta) {
            Vec3 answer;
            const double c = cos(theta);
            const double s = sin(theta);

            answer.x = v.x;
            answer.y = c * v.y - s * v.z;
            answer.z = s * v.y + c * v.z;

            return answer;
        }

        inline static Vec3 rotateY(const Vec3& v, const double& theta) {
            Vec3 answer;
            const double c = cos(theta);
            const double s = sin(theta);

            answer.x = c * v.x + s * v.z;
            answer.y = v.y;
            answer.z = -s * v.x + c * v.z;

            return answer;
        }

        inline static Vec3 rotateZ(const Vec3& v, const double& theta) {
            Vec3 answer;
            const double c = cos(theta);
            const double s = sin(theta);

            answer.x = c * v.x - s * v.y;
            answer.y = s * v.x + c * v.y;
            answer.z = v.z;

            return answer;
        }

        inline bool nearlyEqual(const Vec3& v) {
            if (H_EPSILON < fabs(x - v.x)) return false;
            if (H_EPSILON < fabs(y - v.y)) return false;
            if (H_EPSILON < fabs(z - v.z)) return false;
            return true;
        }

        inline static Vec3 convertVectorRelativeToN(const Vec3& ans, const Vec3& n) {
            //convert coordinate, Duff 2017
            const double s = copysign(1.0, n.z);
            const double a = -1.0 / (s + n.z);
            const double b = n.x * n.y * a;
            const Vec3 u(1.0 + s * n.x * n.x * a, s * b, -s * n.x);
            const Vec3 v(b, s + n.y * n.y * a, -n.y);
            return (u * ans.x + n * ans.y + v * ans.z).normalize();
        }
    };
}
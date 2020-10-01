#pragma once

#include <string>
#include "../Vec3.h"

namespace hiraishi {
    struct Material
    {
        Material() {}
        Material(std::string _name) : name(_name) {}
        Material(const Material& m) {
            name = m.name;
            Ns = m.Ns;
            Ni = m.Ni;
            Tr = m.Tr;
            d = m.d;
            illum = m.illum;
            Ka = m.Ka;
            Kd = m.Kd;
            Ks = m.Ks;
            Ke = m.Ke;
            Tf = m.Tf;
        }
        Material(const std::string _name, const double _Ns, const double _Ni, const double _Tr, const double _d, const int _illum, const Vec3 _Ka, const Vec3 _Kd, const Vec3 _Ks, const Vec3 _Ke, const Vec3 _Tf) {
            name = _name;
            Ns = _Ns;
            Ni = _Ni;
            Tr = _Tr;
            d = _d;
            illum = _illum;
            Ka = _Ka;
            Kd = _Kd;
            Ks = _Ks;
            Ke = _Ke;
            Tf = _Tf;
        }
        ~Material() {}

        std::string name;
        double Ns = 0.0;
        double Ni = 0.0;
        double Tr = 0.0;
        double d = 0.0;
        int illum = -1;
        Vec3 Ka = Vec3(0.0, 0.0, 0.0);
        Vec3 Kd = Vec3(0.0, 0.0, 0.0);
        Vec3 Ks = Vec3(0.0, 0.0, 0.0);
        Vec3 Ke = Vec3(0.0, 0.0, 0.0);
        Vec3 Tf = Vec3(0.0, 0.0, 0.0);
        double roughness = 0.0;
        double anisotopic = 0.0;

        Material& operator=(const Material& m) {
            name = m.name;
            Ns = m.Ns;
            Ni = m.Ni;
            Tr = m.Tr;
            d = m.d;
            illum = m.illum;
            Ka = m.Ka;
            Kd = m.Kd;
            Ks = m.Ks;
            Ke = m.Ke;
            Tf = m.Tf;
            return *this;
        }
    };
}
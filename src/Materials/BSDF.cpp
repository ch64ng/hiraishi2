#include <cassert>
#include "../Vec3.h"
#include "../Spectrum.h"
#include "../Trigonometric.h"
#include "../Random.h"
#include "Material.h"
#include "../Ray.h"
#include "../Intersect.h"
#include "BSDF.h"

using namespace hiraishi;
using namespace Trig;

Vec3 BSDF::randomReflectDir(Random& rng) const { // Hemisphere
    const double r = sqrt(rng.next());
    const double theta = 2.0 * M_PI * rng.next();
    const double x = r * cos(theta);
    const double z = r * sin(theta);
    const double y = sqrt(fmax(0.0, 1.0 - x * x - z * z));
    return Vec3(x, y, z);
}

Vec3 BSDF::randomDirSphere(Random& rng) { // Sphere
    const double theta = 2.0 * M_PI * rng.next();
    const double phi = acos(1.0 - 2.0 * rng.next());
    const double x = sin(phi) * cos(theta);
    const double z = sin(phi) * sin(theta);
    const double y = cos(phi);
    return Vec3(x, y, z);
}

Vec3 BSDF::mirrorReflectDir(const Vec3& n, const Vec3& dir) const {
    return dir - (n * Vec3::dot(n, dir) * 2.0);
}

Vec3 BSDF::refractDir(const Vec3& n, const Vec3& dir, const double ior) const {
    const Vec3 dir_n = dir + n;
    const double ni = ior;
    const double k = 1.0 / sqrt(ni * ni * dir.length() * dir.length() - dir_n.length() * dir_n.length());
    const Vec3 T = (n + dir) * k - n;
    return T;
}

Vec3 BSDF::fresnelDir(const Vec3& n, const Vec3& dir, const double n1, const double n2, const double p) const {
    const double dot = Vec3::dot(n, -dir);
    const bool isTransmitted = (dot < H_EPSILON) ? true : false;
    const Vec3 normal = isTransmitted ? -n : n;
    const double ior = isTransmitted ? n2 / n1 : n1 / n2;
    const double t1 = Vec3::dot(-dir, normal);
    const double t2 = 1.0 - ior * ior * (1.0 - t1 * t1);
    if (t2 < 0.0) { // ‘S”½ŽË
        return mirrorReflectDir(normal, dir);
    }
    const Vec3 wt = (normal * t1 - -dir) * ior - normal * sqrt(t2);
    const double cos = isTransmitted ? Vec3::dot(wt, n) : Vec3::dot(-dir, n);
    const double Fr = fresnelSchlickApprox(ior, cos);
    if (p < Fr) {
        return mirrorReflectDir(normal, dir);
    }
    return wt;
}

double BSDF::fresnelSchlickApprox(const double ior, const double cosd) const {
    const double rF0 = 0.99999 < ior ? 0.0 : (1.0 - ior) / (1.0 + ior);
    const double F0 = rF0 * rF0;
    return F0 + (1.0 - F0) * pow(1.0 - cosd, 5);
}

// Specular BRDF F term
Vec3 BSDF::evaluateF(const Vec3& w, const Vec3& wm) const {
    const double Fr = fresnelSchlickApprox(0.0, Vec3::dot(w, wm));
    return Vec3(Fr);
}

// Specular BRDF G term
double BSDF::smithGGXG1(const Vec3& w) const {
    const double absTanTheta = abs(tanTheta(w));
    if (H_INFINITE < absTanTheta) return 0.0;
    const double a = sqrt(cos2Phi(w) * ax * ax + sin2Phi(w) * ay * ay);
    const double aTan = a * absTanTheta;
    const double lambda = 0.5 * (-1.0 + sqrt(aTan * aTan + 1.0));
    return 1.0 / (1.0 + lambda);
}

double BSDF::evaluateG(const Vec3& wo, const Vec3& wi, const Vec3& wm) const {
    const double G1i = smithGGXG1(wi);
    const double G1o = smithGGXG1(wo);
    const double G = G1i * G1o;
    return G;
}

// Specular BRDF D term
double BSDF::evaluateD(const Vec3& wm) const {
    const double dotHX = wm.x;
    const double dotHY = wm.z;
    const double dotHN = wm.y;
    const double denom = (dotHX * dotHX / (ax * ax)) + (dotHY * dotHY / (ay * ay)) + dotHN * dotHN;
    return 1.0 / (M_PI * ax * ay * (denom * denom));
}

Vec3 BSDF::DisneyBRDF(double& pdf) const {
    const Vec3 normalLocal(0.0, 1.0, 0.0);
    const Vec3 woLocal = Vec3::convertVectorRelativeToN(-ray.d, normalLocal);
    const Vec3 wiLocal = Vec3::convertVectorRelativeToN(isect.wi, normalLocal);
    const Vec3 wmLocal = (woLocal + wiLocal).normalize();
    const Vec3 F = evaluateF(woLocal, wmLocal);
    const double G = evaluateG(woLocal, wiLocal, wmLocal);
    const double D = evaluateD(wmLocal);
    const double dotHL = Vec3::absDot(wmLocal, wiLocal);
    pdf = (G * abs(cosTheta(wiLocal)) / dotHL) * D / (4.0 * dotHL);
    return F * G * D / (4.0 * Vec3::dot(woLocal, wmLocal) * Vec3::dot(wiLocal, wmLocal));
}

// Sample Microfacet Normal(wm)
Vec3 BSDF::sampleMicrofacetNormal(Random& rng) {
    const double r1 = rng.next();
    const double r2 = rng.next();
    const double roughness = isect.mtlPtr->roughness;
    const double aspect = sqrt(1.0 - 0.9 * isect.mtlPtr->anisotopic);
    ax = roughness * roughness / aspect;
    ay = roughness * roughness * aspect;
    // orthonormal basis(y up)
    const Vec3 n(0.0, 1.0, 0.0);
    const Vec3 x(1.0, 0.0, 0.0);
    const Vec3 z(0.0, 0.0, 1.0);
    const Vec3 wm = (x * (ax * cos(2.0 * M_PI * r1)) + (z * (ay * sin(2.0 * M_PI * r1)))) * sqrt(r2 / (1.0 - r2)) + n;
    return Vec3::convertVectorRelativeToN(wm, isect.normal);
}

Vec3 BSDF::evaluateDirection(Random& rng) {
    Vec3 dir(0.0, 0.0, 0.0);
    if (isect.mtlPtr->illum == 2) { // diffuse
        const Vec3 dirLocal = randomReflectDir(rng);
        dir = Vec3::convertVectorRelativeToN(dirLocal, isect.normal);
    }
    else if (isect.mtlPtr->illum == 5) { // mirror
        if (isect.mtlPtr->roughness == 0.0) {
            dir = mirrorReflectDir(isect.normal, ray.d);
        }
        else {
            isect.wm = sampleMicrofacetNormal(rng);
            isect.wi = mirrorReflectDir(dir, isect.wm);
            dir = isect.wi;
        }
    }
    else if (isect.mtlPtr->illum == 7) { // glass
        const double n1 = 1.0; // air
        const double n2 = isect.mtlPtr->Ni;
        dir = fresnelDir(isect.normal, ray.d, n1, n2, rng.next());
    }
    return dir;
}

Vec3 BSDF::evaluateBSDF(double& pdf) const {
    Vec3 fs(1.0, 1.0, 1.0);
    if (isect.mtlPtr->illum == 2) { // diffuse
        fs = isect.mtlPtr->Kd / M_PI;
        pdf = cosTerm / M_PI;
    }
    else if (isect.mtlPtr->illum == 5) { // specular
        // todo
        if (isect.mtlPtr->roughness == 0.0) {
            pdf = cosTerm;
        }
        else {
            fs = DisneyBRDF(pdf);
        }
    }
    else if (isect.mtlPtr->illum == 7) { // glass
        // todo
        fs = fs * isect.mtlPtr->Tf;
        pdf = cosTerm;
    }
    return fs;
}

Vec3 BSDF::evaluateSpectrumDirection(Random& rng) {
    Vec3 dir(0.0, 0.0, 0.0);
    if (isect.mtlPtr->illum == 2) { // diffuse
        const Vec3 dirLocal = randomReflectDir(rng);
        dir = Vec3::convertVectorRelativeToN(dirLocal, isect.normal);
    }
    else if (isect.mtlPtr->illum == 5) { // mirror
        dir = mirrorReflectDir(isect.normal, ray.d);
    }
    else if (isect.mtlPtr->illum == 7) { // glass
        const double n1 = 1.0; // air
        const double n2 = isect.mtlPtr->Ni;
        dir = fresnelDir(isect.normal, ray.d, n1, n2, rng.next());
    }
    return dir;
}

Spectrum BSDF::evaluateSpectrumBSDF(double& pdf, const Spectrum& kd) const {
    Spectrum fs(1.0);
    if (isect.mtlPtr->illum == 2) { // diffuse
        fs = kd / M_PI;
        pdf = cosTerm / M_PI;
    }
    else if (isect.mtlPtr->illum == 5) { // specular
        // todo
        pdf = cosTerm;
    }
    else if (isect.mtlPtr->illum == 7) { // glass
        // todo
        pdf = cosTerm;
    }
    return fs;
}
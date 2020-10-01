#include <iostream>
#include <string>
#include <chrono>
#include <omp.h>
#include "../Constant.h"
#include "../Random.h"
#include "../Vec3.h"
#include "../Spectrum.h"
#include "../Sampler.h"
#include "../Camera.h"
#include "../Materials/Material.h"
#include "../Ray.h"
#include "../BBox.h"
#include "../Face.h"
#include "../Sphere.h"
#include "../Intersect.h"
#include "../Materials/BSDF.h"
#include "../Accelerator/KdTree.h"
#include "../ModelSet.h"
#include "../Film.h"
#include "../Scene.h"
#include "Renderer.h"
#include "Renderer_SingleSpectrum_PT.h"

using namespace hiraishi;

void Renderer_SingleSpectrum_PT::render(const Scene* scene, const Camera* camera, Film* film) {
    initSpectrum();
    model = scene->getModel();

    light.setSpectrum(CIEStandardIlluminantD65);
    light = Spectrum::map(light, 0.0, 25.0);

    const auto start = std::chrono::system_clock::now();
    std::cout << ">> Render : START" << std::endl;
    int numFinished = 0;

#pragma omp parallel for schedule(dynamic, 1)
    for (int j = 0; j < film->height; j++) {
        thread_local Random rng(42 + omp_get_thread_num());
        for (int i = 0; i < film->width; i++) {
            renderPixel(scene, camera, film, i, j, rng);
        }
#pragma omp atomic
        numFinished += 100;
#pragma omp critical
        fprintf(stderr, ">> Render : %d %%\r", numFinished / film->height);
    }

    const auto end = std::chrono::system_clock::now();
    const auto duration = end - start;
    const auto msec = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << std::endl << ">> Render : FINISH" << std::endl
        << ">> Render : Time " << msec << "msec" << std::endl << std::endl;
    film->setRenderStatus(msec, spp);
}

void Renderer_SingleSpectrum_PT::renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng) {
    const int p = i + film->width * j;
    const Vec3& eye = camera->getEye();
    Sampler sampler;
    Spectrum averageSpectrum(0.0);
    int numLambdaSamples[60] = { 0 };

    for (int s = 0; s < spp; s++) {
        // Sample pos on film and generate initial ray
        const Vec3 target = camera->samplePixel(i, j, film->width, film->height, rng, sampler);
        Ray ray(eye, target - eye);

        // Initialize variables
        double spectrum(0.0);
        double throughput(1.0);
        int lambdaIdx = (int)(rng.next() * 59.999);

        int bounce = 0;
        Intersect isect = scene->intersect(ray, rng);
        Intersect prevIsect;

        // Main Rendering Loop
        while (true) {
            prevIsect = isect;

            // Intersect with model
            isect = scene->intersect(ray, rng);
            if (isect.t == H_INFINITE) {
                #if 0 // use skylight if true
                const Vec3 sunPos(100.0, 10.0, -5.0);
                const Sphere sun(sunPos, 10.0);
                double sunt = H_INFINITE;
                const bool isSunIsect = sun.intersect(ray, &sunt);
                const double sunIntensity = 100.0;

                if (isSunIsect) {
                    const double ke = Spectrum::map(light, 0.0, sunIntensity).c[lambdaIdx];
                    spectrum = spectrum + ke * throughput;
                }
                else {
                    const double t = ray.d.y * 0.5 + 0.5;
                    const Vec3 c = Vec3(1.0, 1.0, 1.0) * (1.0 - t) + Vec3(0.5, 0.7, 1.0) * t;
                    const Vec3 RGBcolor = Vec3(std::min(std::max(c.x, 0.0), 1.0), std::min(std::max(c.y, 0.0), 1.0), std::min(std::max(c.z, 0.0), 1.0));
                    const double s = Spectrum::map(RGB2Spectrum(RGBcolor, 1), 0.0, 1.0).c[lambdaIdx];
                    spectrum = spectrum + s * throughput;
                }

                #endif

                break;
            }

            // Add Le
            if (isect.mtlPtr->Ke != Vec3::black()) {
                const double ke = light.c[lambdaIdx];
                spectrum = spectrum + ke * throughput;
            }

            // Set next ray and update throughput
            const double cosTerm = Vec3::absDot(ray.d, isect.normal);
            BSDF bsdf = BSDF(ray, isect, cosTerm);
            const Vec3 dir = bsdf.evaluateDirection(rng);
            double pdf = 1.0;
            const Spectrum kd = RGB2Spectrum(isect.mtlPtr->Kd, 0);
            const Spectrum fs = bsdf.evaluateSpectrumBSDF(pdf, kd);
            ray = Ray(isect.pos, dir);
            throughput = throughput * fs.c[lambdaIdx] * cosTerm / pdf;

            // Russian Roulette
            const double prob = throughput;
            if (prob == 0) break;
            if (prob < rng.next() || maxBounce < bounce) {
                break;
            }
            else {
                throughput = throughput / prob;
            }

            bounce++;
        }
        averageSpectrum.c[lambdaIdx] = averageSpectrum.c[lambdaIdx] + spectrum;
        numLambdaSamples[lambdaIdx]++;
    }
    for (int l = 0; l < 60; l++) {
        if (numLambdaSamples[l] == 0) continue;
        averageSpectrum.c[l] = averageSpectrum.c[l] * (1.0 / numLambdaSamples[l]);
    }
    film->setPixelColor(Spectrum2RGB(averageSpectrum), p);
}

void Renderer_SingleSpectrum_PT::initSpectrum() {
    // compute XYZ matching functions
    for (int i = 0; i < numSpectralSamples; ++i) {
        double l0 = H_Math::lerp((double)i / (double)numSpectralSamples, sampledLambdaStart, sampledLambdaEnd);
        double l1 = H_Math::lerp(((double)i + 1.0) / (double)numSpectralSamples, sampledLambdaStart, sampledLambdaEnd);
        X.c[i] = averageSpectrumSamples(CIE_lambda, CIE_X, numCIESamples, l0, l1);
        Y.c[i] = averageSpectrumSamples(CIE_lambda, CIE_Y, numCIESamples, l0, l1);
        Z.c[i] = averageSpectrumSamples(CIE_lambda, CIE_Z, numCIESamples, l0, l1);
    }
    // compute RGB to spectrum functions
    for (int i = 0; i < numSpectralSamples; ++i) {
        double l0 = H_Math::lerp((double)i / (double)numSpectralSamples, sampledLambdaStart, sampledLambdaEnd);
        double l1 = H_Math::lerp(((double)i + 1.0) / (double)numSpectralSamples, sampledLambdaStart, sampledLambdaEnd);
        rgbRefl2SpectWhite.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectWhiteData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectCyan.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectCyanData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectMagenta.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectMagentaData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectYellow.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectYellowData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectRed.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectRedData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectGreen.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectGreenData, numRGB2SpectSamples, l0, l1);
        rgbRefl2SpectBlue.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBRefl2SpectBlueData, numRGB2SpectSamples, l0, l1);

        rgbIllum2SpectWhite.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectWhiteData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectCyan.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectCyanData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectMagenta.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectMagentaData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectYellow.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectYellowData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectRed.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectRedData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectGreen.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectGreenData, numRGB2SpectSamples, l0, l1);
        rgbIllum2SpectBlue.c[i] = averageSpectrumSamples(RGB2SpectLambda, RGBIllum2SpectBlueData, numRGB2SpectSamples, l0, l1);
    }
}

Spectrum Renderer_SingleSpectrum_PT::RGB2Spectrum(const Vec3 rgb, int type) {
    Spectrum answer;
    if (type == 0) { // 0 -> reflectance
        // Convert reflectance spectrum to RGB
        if (rgb.x <= rgb.y && rgb.x <= rgb.z) {
            // Compute reflectance _SampledSpectrum_ with _rgb.x_ as minimum
            answer += rgbRefl2SpectWhite * rgb.x;
            if (rgb.y <= rgb.z) {
                answer += rgbRefl2SpectCyan * (rgb.y - rgb.x);
                answer += rgbRefl2SpectBlue * (rgb.z - rgb.y);
            }
            else {
                answer += rgbRefl2SpectCyan * (rgb.z - rgb.x);
                answer += rgbRefl2SpectGreen * (rgb.y - rgb.z);
            }
        }
        else if (rgb.y <= rgb.x && rgb.y <= rgb.z) {
            // Compute reflectance _SampledSpectrum_ with _rgb.y_ as minimum
            answer += rgbRefl2SpectWhite * rgb.y;
            if (rgb.x <= rgb.z) {
                answer += rgbRefl2SpectMagenta * (rgb.x - rgb.y);
                answer += rgbRefl2SpectBlue * (rgb.z - rgb.x);
            }
            else {
                answer += rgbRefl2SpectMagenta * (rgb.z - rgb.y);
                answer += rgbRefl2SpectRed * (rgb.x - rgb.z);
            }
        }
        else {
            // Compute reflectance _SampledSpectrum_ with _rgb.z_ as minimum
            answer += rgbRefl2SpectWhite * rgb.z;
            if (rgb.x <= rgb.y) {
                answer += rgbRefl2SpectYellow * (rgb.x - rgb.z);
                answer += rgbRefl2SpectGreen * (rgb.y - rgb.x);
            }
            else {
                answer += rgbRefl2SpectYellow * (rgb.y - rgb.z);
                answer += rgbRefl2SpectRed * (rgb.x - rgb.y);
            }
        }
        answer *= 0.94;
    }
    else { // 1 -> illuminance
        // Convert illuminant spectrum to RGB
        if (rgb.x <= rgb.y && rgb.x <= rgb.z) {
            // Compute illuminant _SampledSpectrum_ with _rgb.x_ as minimum
            answer += rgbIllum2SpectWhite * rgb.x;
            if (rgb.y <= rgb.z) {
                answer += rgbIllum2SpectCyan * (rgb.y - rgb.x);
                answer += rgbIllum2SpectBlue * (rgb.z - rgb.y);
            }
            else {
                answer += rgbIllum2SpectCyan * (rgb.z - rgb.x);
                answer += rgbIllum2SpectGreen * (rgb.y - rgb.z);
            }
        }
        else if (rgb.y <= rgb.x && rgb.y <= rgb.z) {
            // Compute illuminant _SampledSpectrum_ with _rgb.y_ as minimum
            answer += rgbIllum2SpectWhite * rgb.y;
            if (rgb.x <= rgb.z) {
                answer += rgbIllum2SpectMagenta * (rgb.x - rgb.y);
                answer += rgbIllum2SpectBlue * (rgb.z - rgb.x);
            }
            else {
                answer += rgbIllum2SpectMagenta * (rgb.z - rgb.y);
                answer += rgbIllum2SpectRed * (rgb.x - rgb.z);
            }
        }
        else {
            // Compute illuminant _SampledSpectrum_ with _rgb.z_ as minimum
            answer += rgbIllum2SpectWhite * rgb.z;
            if (rgb.x <= rgb.y) {
                answer += rgbIllum2SpectYellow * (rgb.x - rgb.z);
                answer += rgbIllum2SpectGreen * (rgb.y - rgb.x);
            }
            else {
                answer += rgbIllum2SpectYellow * (rgb.y - rgb.z);
                answer += rgbIllum2SpectRed * (rgb.x - rgb.y);
            }
        }
        answer *= 0.86445;
    }
    return answer.clamp();
}

Vec3 Renderer_SingleSpectrum_PT::Spectrum2RGB(const Spectrum& s) {
    // convert Spectrum to XYZ Color
    Vec3 XYZ(0.0, 0.0, 0.0);
    for (int i = 0; i < numSpectralSamples; ++i) {
        XYZ.x += X.c[i] * s.c[i];
        XYZ.y += Y.c[i] * s.c[i];
        XYZ.z += Z.c[i] * s.c[i];
    }
    double scale = (double)(sampledLambdaEnd - sampledLambdaStart) / (double)(CIE_Y_integral * numSpectralSamples);
    XYZ = XYZ * scale;

    // convert XYZ Color to RGB Color
    Vec3 RGB(0.0, 0.0, 0.0);
    RGB.x = 3.240479 * XYZ.x - 1.537150 * XYZ.y - 0.498535 * XYZ.z;
    RGB.y = -0.969256 * XYZ.x + 1.875991 * XYZ.y + 0.041556 * XYZ.z;
    RGB.z = 0.055648 * XYZ.x - 0.204043 * XYZ.y + 1.057311 * XYZ.z;
    return Vec3::clamp(RGB);
}

double Renderer_SingleSpectrum_PT::averageSpectrumSamples(const double* lambda, const double* vals, int n, double lambdaStart, double lambdaEnd) {
    if (lambdaEnd <= lambda[0])       return vals[0];
    if (lambda[n - 1] <= lambdaStart) return vals[n - 1];
    if (n == 1) return vals[0];

    double sum = 0.0;
    if (lambdaStart < lambda[0])   sum += vals[0] * (lambda[0] - lambdaStart);
    if (lambda[n - 1] < lambdaEnd) sum += vals[n - 1] * (lambdaEnd - lambda[n - 1]);
    int i = 0;
    while (lambda[i + 1] < lambdaStart) ++i;
    auto interp = [lambda, vals](double w, int i) {
        return H_Math::lerp((w - lambda[i]) / (lambda[i + 1] - lambda[i]), vals[i], vals[i + 1]);
    };
    for (; i + 1 < n && lambda[i] <= lambdaEnd; ++i) {
        double segLambdaStart = std::max(lambdaStart, lambda[i]);
        double segLambdaEnd = std::min(lambdaEnd, lambda[i + 1]);
        sum += 0.5 * (interp(segLambdaStart, i) + interp(segLambdaEnd, i)) * (segLambdaEnd - segLambdaStart);
    }
    return sum / (lambdaEnd - lambdaStart);
}

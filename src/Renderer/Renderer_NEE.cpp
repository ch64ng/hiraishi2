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
#include "Renderer_NEE.h"

using namespace hiraishi;

void Renderer_NEE::render(const Scene* scene, const Camera* camera, Film* film) {
    model = scene->getModel();

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

void Renderer_NEE::renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng) {
    const int p = i + film->width * j;
    const Vec3& eye = camera->getEye();
    Sampler sampler;
    Vec3 averageColor(0.0, 0.0, 0.0);
    for (int s = 0; s < spp; s++) {
        // Sample pos on film and generate initial ray
        const Vec3 target = camera->samplePixel(i, j, film->width, film->height, rng, sampler);
        Ray ray(eye, target - eye);

        // Initialize variables
        Vec3 color(0.0, 0.0, 0.0);
        Vec3 throughput(1.0, 1.0, 1.0);
        int bounce = 0;

        // initial ray contribution
        Intersect isect = scene->intersect(ray, rng);
        if (isect.t == H_INFINITE) break;
        if (isect.mtlPtr->Ke != Vec3::black()) {
            color = color + isect.mtlPtr->Ke * throughput;
        }

        // Main Rendering Loop
        while (true) {
            const int previousMtl = isect.mtlPtr->illum;

            // Intersect with model
            isect = scene->intersect(ray, rng);
            if (isect.t == H_INFINITE) break;

            // Add Le
            if (isect.mtlPtr->Ke != Vec3::black()) {
                if (previousMtl == H_MTL_MIRROR || previousMtl == H_MTL_GLASS) {
                    color = color + isect.mtlPtr->Ke * throughput;
                }
            }

            // NEE
            if (isect.mtlPtr->illum != H_MTL_GLASS && isect.mtlPtr->illum != H_MTL_MIRROR) {
                const Vec3 lp = model.randomPosOnLight(rng); // currently cannnot use
                const Ray shadowRay(isect.pos, lp - isect.pos);
                Intersect shadowIsect = scene->intersect(shadowRay, rng);
                const Vec3 kd = isect.mtlPtr->Kd;

                if (shadowIsect.mtlPtr != NULL && shadowIsect.mtlPtr->Ke != Vec3::black()) {
                    const double dot1 = Vec3::dot(isect.normal, shadowRay.d);
                    const double dot2 = Vec3::dot(shadowIsect.normal, -shadowRay.d);
                    if (0.0 < dot1 && 0.0 < dot2) {
                        const double G = dot1 * dot2 / Vec3(lp - isect.pos).length2();
                        const double pd = 1.0 / model.getLightArea();
                        color = color + throughput * shadowIsect.mtlPtr->Ke * ((kd / M_PI) * G) / pd;
                    }
                }
            }

            // Set next ray and update throughput
            const double cosTerm = Vec3::absDot(ray.d, isect.normal);
            BSDF bsdf = BSDF(ray, isect, cosTerm);
            const Vec3 dir = bsdf.evaluateDirection(rng);
            double pdf = 1.0;
            const Vec3 fs = bsdf.evaluateBSDF(pdf);
            ray = Ray(isect.pos, dir);
            throughput = throughput * fs * cosTerm / pdf;

            // Russian Roulette
            const double prob = fmax(throughput.x, fmax(throughput.y, throughput.z));
            if (prob == 0) break;
            if (prob < rng.next() || maxBounce < bounce) {
                break;
            }
            else {
                throughput = throughput / prob;
            }

            bounce++;
        }
        averageColor = averageColor + color;
    }
    averageColor = averageColor * (1.0 / spp);
    film->setPixelColor(averageColor, p);
}
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
#include "Renderer_PT_Volume.h"

using namespace hiraishi;

void Renderer_PT_Volume::render(const Scene* scene, const Camera* camera, Film* film) {
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

void Renderer_PT_Volume::renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng) {
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
        bool volume = false;
        const bool volumeAtmosphere = false;
        Intersect isect;

        // Main Rendering Loop
        while (true) {
            // Intersect with model
            isect = scene->intersect(ray, rng);
            if (isect.t == H_INFINITE) {
                #if 0 // use skylight if true
                    const double t = ray.d.y * 0.5 + 0.5;
                    const Vec3 c = Vec3(1.0, 1.0, 1.0) * (1.0 - t) + Vec3(0.5, 0.7, 1.0) * t;
                    color = Vec3(min(max(c.x, 0.0), 1.0), min(max(c.y, 0.0), 1.0), min(max(c.z, 0.0), 1.0));
                #endif
                break;
            }
            if (isect.mtlPtr->illum == 11) volume = true;

            if (volume) {
                // Volume properties
                const double sigma_a = 0.01;                        // absorption coefficient
                const double sigma_s = 1.0;                         // scattering coefficient
                const double sigma_t = sigma_a + sigma_s;           // extinction coefficient
                const double scatterAlbedo = sigma_s / sigma_t;     // scattering albedo
                const double majorant = 2.0;
                const double sigma_n = majorant - sigma_t;          // null-collision coefficient

                // Free tracking

                // sample scatter distance
                const double v_t = -log(1.0 - rng.next()) / sigma_t;

                if (isect.t < v_t) {
                    // Set next ray and update throughput
                    const double cosTerm = Vec3::absDot(ray.d, isect.normal);
                    BSDF bsdf = BSDF(ray, isect, cosTerm);
                    double pdf = 1.0;
                    const Vec3 fs = bsdf.evaluateBSDF(pdf);
                    ray = Ray(isect.pos, ray.d);
                    throughput = throughput * fs * cosTerm / pdf;
                    // Add Le
                    if (isect.mtlPtr->Ke != Vec3::black()) {
                        color = color + isect.mtlPtr->Ke * throughput;
                    }

                    // Russian Roulette
                    const double prob = fmax(throughput.x, fmax(throughput.y, throughput.z));
                    if (prob == 0) break;
                    if (prob < rng.next() || maxBounce < bounce) {
                        break;
                    }
                    else {
                        throughput = throughput / prob;
                    }

                    volume = false;

                    bounce++;

                    continue;
                }

                // update ray
                ray.o = ray.o + ray.d * v_t;

                if (rng.next() < sigma_a / majorant) { // absorption or emission collision
                    throughput = throughput * 0.97;
                    break;
                }
                else if (rng.next() < 1.0 - sigma_n / majorant) { // scattering collision
                    ray.d = BSDF::randomDirSphere(rng);
                    isect = scene->intersect(ray, rng);
                    double v_Tr = exp(-sigma_t * v_t);
                    throughput = throughput * v_Tr;
                }
                else { // null collision

                }

                // Russian Roulette
                if (scatterAlbedo < rng.next()) break;

            } // end of if(volume == true)
            else {
                if (volumeAtmosphere) { 
                    // Volume properties
                    const double sigma_a = 0.01;                        // absorption coefficient
                    const double sigma_s = 1.5;                         // scattering coefficient
                    const double sigma_t = sigma_a + sigma_s;           // extinction coefficient
                    const double scatterAlbedo = sigma_s / sigma_t;     // scattering albedo
                    const double majorant = 2.0;
                    const double sigma_n = majorant - sigma_t;          // null-collision coefficient

                    // Free tracking

                    // sample scatter distance
                    const double v_t = -log(1.0 - rng.next()) / sigma_t;

                    if (isect.t < v_t) {
                        // Set next ray and update throughput
                        const double cosTerm = Vec3::absDot(ray.d, isect.normal);
                        BSDF bsdf = BSDF(ray, isect, cosTerm);
                        const Vec3 dir = bsdf.evaluateDirection(rng);
                        double pdf = 1.0;
                        const Vec3 fs = bsdf.evaluateBSDF(pdf);
                        ray = Ray(isect.pos, dir);
                        throughput = throughput * fs * cosTerm / pdf;
                        // Add Le
                        if (isect.mtlPtr->Ke != Vec3::black()) {
                            color = color + isect.mtlPtr->Ke * throughput;
                        }
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

                        continue;
                    }

                    // update ray
                    ray.o = ray.o + ray.d * v_t;

                    if (rng.next() < sigma_a / majorant) { // absorption or emission collision
                        throughput = throughput * 0.97;
                        break;
                    }
                    else if (rng.next() < 1.0 - sigma_n / majorant) { // scattering collision
                        ray.d = BSDF::randomDirSphere(rng);
                        isect = scene->intersect(ray, rng);
                        double v_Tr = exp(-sigma_t * v_t);
                        throughput = throughput * v_Tr;
                    }
                    else { // null collision

                    }

                    // Russian Roulette
                    if (scatterAlbedo < rng.next()) break;

                }
                else { // Vanilla PT
                    // Add Le
                    if (isect.mtlPtr->Ke != Vec3::black()) {
                        color = color + isect.mtlPtr->Ke * throughput;
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
            }
        }
        averageColor = averageColor + color;
    }
    averageColor = averageColor * (1.0 / spp);
    film->setPixelColor(averageColor, p);
}
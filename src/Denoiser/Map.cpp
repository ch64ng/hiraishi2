#include <fstream>
#include <string>
#include <numeric>
#include "../Vec3.h"
#include "Map.h"

using namespace hiraishi;

void Map::init(const int _width, const int _height, const int _spp) {
    width = _width;
    height = _height;
    numPixels = width * height;
    spp = _spp;
    for (int i = 0; i < numAuxMaps; i++) {
        auxMaps[i] = new unsigned char[numPixels * 3];
        for (int j = 0; j < numPixels; j++) {
            auxMaps[i][j * 3] = 0;
            auxMaps[i][j * 3 + 1] = 0;
            auxMaps[i][j * 3 + 2] = 0;
        }
    }
    lightHitMap.resize(numPixels, 1);
}

void Map::setNormal(const Vec3& n, const int p) {
    auxMaps[0][p * 3] = (unsigned char)(fmin(255.0, (n.x * 0.5 + 0.5) * 255.0));
    auxMaps[0][p * 3 + 1] = (unsigned char)(fmin(255.0, (n.y * 0.5 + 0.5) * 255.0));
    auxMaps[0][p * 3 + 2] = (unsigned char)(fmin(255.0, (n.z * 0.5 + 0.5) * 255.0));
}

void Map::setDepth(const double d, const int p) {
    const double depth = fmin(255.0, (d + minDepth) / depthRange * 255.0);
    auxMaps[1][p * 3] = (unsigned char)depth;
    auxMaps[1][p * 3 + 1] = (unsigned char)depth;
    auxMaps[1][p * 3 + 2] = (unsigned char)depth;
}

void Map::setVisibility(const int p) {
    const int n = (int)(auxMaps[2][p * 3] + (255.0 / spp));
    auxMaps[2][p * 3] = (unsigned char)n;
    auxMaps[2][p * 3 + 1] = (unsigned char)n;
    auxMaps[2][p * 3 + 2] = (unsigned char)n;
}

void Map::setAlbedo(const Vec3& a, const int p) {
    auxMaps[3][p * 3] = (unsigned char)(fmin(255.0, a.x * 255.0));
    auxMaps[3][p * 3 + 1] = (unsigned char)(fmin(255.0, a.y * 255.0));
    auxMaps[3][p * 3 + 2] = (unsigned char)(fmin(255.0, a.z * 255.0));
}

void Map::setLightHit(const int p) {
    lightHitMap.at(p) += 1;
}

void Map::normalizeLH() {
    int maxVal = 0;
    for (int i = 0; i < numPixels; i++) {
        maxVal = fmax((int)lightHitMap[i], maxVal);
    }
    const double scale = fmax(1.0, 255.0 / (double)maxVal);
    for (int i = 0; i < numPixels; i++) {
        const int n = (double)lightHitMap[i] * scale;
        lightHitMap[i] = n;
    }
}

void Map::optimizeLH() {
    if (spp < 255) {
        for (auto& numHit : lightHitMap) {
            if (spp <= numHit) numHit = 255;
        }
    }
    const double average = std::accumulate(lightHitMap.begin(), lightHitMap.end(), 0.0) / (double)numPixels;
    const double scale = 0.0 < average ? 127.0 / average : 255.0;
    for (int i = 0; i < numPixels; i++) {
        const double n = fmin(255.0, lightHitMap[i] * scale);
        lightHitMap[i] = (int)n;
    }
}

void Map::writeImage() {
    optimizeLH();
    std::string normalStr = "results/NormalMap.ppm";
    std::string depthStr = "results/DepthMap.ppm";
    std::string visibilityStr = "results/VisibilityMap.ppm";
    std::string albedoStr = "results/AlbedoMap.ppm";
    std::string lightHitStr = "results/LightHitMap.ppm";
    std::ofstream normalOfs(normalStr);
    std::ofstream depthOfs(depthStr);
    std::ofstream visibilityOfs(visibilityStr);
    std::ofstream albedoOfs(albedoStr);
    std::ofstream lightHitOfs(lightHitStr);
    normalOfs << "P3\n" << width << " " << height << "\n255\n";
    depthOfs << "P3\n" << width << " " << height << "\n255\n";
    visibilityOfs << "P3\n" << width << " " << height << "\n255\n";
    albedoOfs << "P3\n" << width << " " << height << "\n255\n";
    lightHitOfs << "P3\n" << width << " " << height << "\n255\n";
    for (int j = 0; j < height; j++) {
        const int k = height - j - 1;
        for (int i = 0; i < width; i++) {
            const int p = (k * width + i) * 3;
            normalOfs << (int)auxMaps[0][p] << " " << (int)auxMaps[0][p + 1] << " " << (int)auxMaps[0][p + 2] << "\n";
            depthOfs << (int)auxMaps[1][p] << " " << (int)auxMaps[1][p + 1] << " " << (int)auxMaps[1][p + 2] << "\n";
            visibilityOfs << (int)auxMaps[2][p] << " " << (int)auxMaps[2][p + 1] << " " << (int)auxMaps[2][p + 2] << "\n";
            albedoOfs << (int)auxMaps[3][p] << " " << (int)auxMaps[3][p + 1] << " " << (int)auxMaps[3][p + 2] << "\n";
            const int pp = k * width + i;
            lightHitOfs << lightHitMap[pp] << " " << lightHitMap[pp] << " " << lightHitMap[pp] << "\n";
        }
    }
}
#include <iostream>
#include <fstream>
#include <string>
#include <direct.h>
#include <sys/stat.h>
#include "Vec3.h"
#include "Mathematics.h"
#include "Film.h"

using namespace hiraishi;

void Film::init() {
    numPixels = width * height;
    pixels = new unsigned char[numPixels * 3];
    for (int i = 0; i < numPixels; i++) {
        pixels[i * 3] = 170;
        pixels[i * 3 + 1] = 170;
        pixels[i * 3 + 2] = 170;
    }
}

void Film::setPixelColor(const Vec3& c, int p) {
    // map color from [0,1] to [0,255] and gamma correction
    const Vec3 color = Vec3::clamp(c);
    pixels[p * 3] = (unsigned char)(fmin(255.0, pow(color.x, 1.0 / 2.2) * 255.0));
    pixels[p * 3 + 1] = (unsigned char)(fmin(255.0, pow(color.y, 1.0 / 2.2) * 255.0));
    pixels[p * 3 + 2] = (unsigned char)(fmin(255.0, pow(color.z, 1.0 / 2.2) * 255.0));
}

void Film::setRenderStatus(const long long _msec, const int _spp) {
    msec = _msec;
    spp = _spp;
}

void Film::writeImage() {
    struct stat statBuf;
    if (stat("results", &statBuf) == -1) _mkdir("results");
    std::string fileName = "results/" + std::to_string(msec) + "msec_" + std::to_string(spp) + "spp.ppm";
    std::ofstream ofs(fileName);
    ofs << "P3\n" << width << " " << height << "\n255\n";
    for (int j = height - 1; 0 <= j; j--) {
        for (int i = 0; i < width; i++) {
            ofs << (int)pixels[(j * width + i) * 3] << " " << (int)pixels[(j * width + i) * 3 + 1] << " " << (int)pixels[(j * width + i) * 3 + 2] << "\n";
        }
    }
    std::cout << ">> OUTPUT IMAGE : FINISH" << std::endl << std::endl;
}

void Film::writePixels() {
    writePixels1("pixelsR.csv", 0);
    writePixels1("pixelsG.csv", 1);
    writePixels1("pixelsB.csv", 2);
}

void Film::writePixels1(std::string filename, int offset) {
    std::ofstream p(filename.c_str());
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            p << static_cast<int>(pixels[(y * width + x) * 3 + offset]);
            if (x == width - 1) break;
            p << ",";
        }
        p << std::endl;
    }
    p.close();
}
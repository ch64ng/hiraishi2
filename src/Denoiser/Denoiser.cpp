#include <iostream>
#include <fstream>
#include <string>
#include "Map.h"
#include "../Film.h"
#include "Filter.h"
#include "Denoiser.h"

using namespace hiraishi;

void Denoiser::init(Film* film) {
    // init variables
    width = film->width;
    height = film->height;
    numPixels = film->getNumPixels();
    // init bufPixels (copy rendered image)
    bufPixels = new unsigned char[numPixels * 3];
    for (int i = 0; i < numPixels; i++) {
        bufPixels[i * 3] = film->getPixel(i * 3);
        bufPixels[i * 3 + 1] = film->getPixel(i * 3 + 1);
        bufPixels[i * 3 + 2] = film->getPixel(i * 3 + 2);
    }
    // init outPixels
    outPixels = new unsigned char[numPixels * 3];
    for (int i = 0; i < numPixels; i++) {
        outPixels[i * 3] = 0;
        outPixels[i * 3 + 1] = 0;
        outPixels[i * 3 + 2] = 0;
    }
    // init filter
    filter.init(width, height, numPixels);
}

void Denoiser::denoise(Film* film, Map* map) {
    std::cout << ">> Denoise : START" << std::endl;
    
    filter.equalize(bufPixels, outPixels, 7);
    writeImage("normalEqualize");

    init(film);

    filter.gaussian(bufPixels, outPixels, 7, 0.5);
    writeImage("gaussian05");

    init(film);

    filter.gaussian(bufPixels, outPixels, 7, 10.0);
    writeImage("gaussian10");

    init(film);

    filter.gaussian(bufPixels, outPixels, 7, 1.0);
    writeImage("gaussian1");

    init(film);

    filter.bilateral(bufPixels, outPixels, 7, 1.0, 10.0, 3);
    writeImage("bilateralIter3");

    init(film);

    filter.jointBilateral(bufPixels, outPixels, 7, 1.0, 10.0, 3, map);
    writeImage("jointBilateralIter3");

    init(film);

    filter.equalizeExact(bufPixels, outPixels, 3, map->getLightHitMap());
    writeImage("equalizeExact");

    init(film);

    filter.equalizeWithLHWeight(bufPixels, outPixels, 3, map->getLightHitMap());
    writeImage("equalizeWithLHWeight");

    init(film);

    filter.median(bufPixels, outPixels, 3);
    writeImage("median");

    std::cout << ">> Denoise : FINISH" << std::endl << std::endl;
}

void Denoiser::writeImage(const char *_name) {
    std::string name = _name;
    std::string fileName = "results/" + name + ".ppm";
    std::ofstream ofs(fileName);
    ofs << "P3\n" << width << " " << height << "\n255\n";
    for (int j = height - 1; 0 <= j; j--) {
        for (int i = 0; i < width; i++) {
            ofs << (int)outPixels[(j * width + i) * 3] << " " << (int)outPixels[(j * width + i) * 3 + 1] << " " << (int)outPixels[(j * width + i) * 3 + 2] << "\n";
        }
    }
}
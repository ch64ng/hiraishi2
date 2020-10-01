#include <fstream>
#include <string>
#include <omp.h>
#include "../Vec3.h"
#include "Map.h"
#include "../Film.h"
#include "Filter.h"

using namespace hiraishi;

void Filter::init(const int _width, const int _height, const int _numPixels) {
    width = _width;
    height = _height;
    numPixels = _numPixels;
}

void Filter::grayscale(unsigned char* inPixels, unsigned char* outPixels) {
#pragma omp parallel for schedule(dynamic, 1)
    for (int i = 0; i < numPixels; i++) {
        int grayscale = ((int)inPixels[i * 3] + (int)inPixels[i * 3 + 1] + (int)inPixels[i * 3 + 2]) / 3;
        outPixels[i * 3] = (unsigned char)grayscale;
        outPixels[i * 3 + 1] = (unsigned char)grayscale;
        outPixels[i * 3 + 2] = (unsigned char)grayscale;
    }
}

void Filter::equalize(unsigned char* inPixels, unsigned char* outPixels, int ksize) { // ksize must be odd number
    const int numElements = 3; //number of elements per pixel
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;
#pragma omp parallel for schedule(dynamic, 1)
    for (int i = initialPixel; i < numPixels; i++) {
        if ((width - offset <= i % width) && (i % width < offset)) continue;
        if (width * (height - offset) < i) continue;
        int average[numElements] = {};
        for (int n = 0; n < numElements; n++) {
            for (int ys = 0; ys < ksize; ys++) {
                // Y pos on kernel
                const int y = (ys - offset) * width * numElements;
                for (int xs = 0; xs < ksize; xs++) {
                    // X pos on kernel
                    const int x = (xs - offset) * numElements;
                    average[n] += (int)inPixels[i * numElements + n + x + y];
                }
            }
            average[n] = average[n] / (ksize * ksize);
        }
        //out
        for (int n = 0; n < numElements; n++) {
            outPixels[i * numElements + n] = (unsigned char)average[n];
        }
    }
}

void Filter::equalizeWithLHWeight(unsigned char* inPixels, unsigned char* outPixels, int ksize, const std::vector<int>& lightHitMap) {
    const int numElements = 3; //number of elements per pixel
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = initialPixel; i < numPixels; i++) {
        if ((width - offset <= i % width) && (i % width < offset)) continue;
        if (width * (height - offset) < i) continue;
        // calculate kernel weight
        int kareaSum = 0;
        for (int ys = 0; ys < ksize; ys++) {
            // Y pos on kernel
            const int y = (ys - offset) * width;
            for (int xs = 0; xs < ksize; xs++) {
                // X pos on kernel
                const int x = xs - offset;
                kareaSum += (int)lightHitMap[i + x + y];
            }
        }

        int average[numElements] = {};
        for (int n = 0; n < numElements; n++) {
            for (int ys = 0; ys < ksize; ys++) {
                // Y pos on kernel
                const int y = (ys - offset) * width * numElements;
                const int yl = (ys - offset) * width;
                for (int xs = 0; xs < ksize; xs++) {
                    // X pos on kernel
                    const int x = (xs - offset) * numElements;
                    const int xl = xs - offset;
                    average[n] += (int)((double)inPixels[i * numElements + n + x + y] * ((double)lightHitMap[i + xl + yl] / (double)kareaSum));
                }
            }
        }
        //out
        for (int n = 0; n < numElements; n++) {
            outPixels[i * numElements + n] = (unsigned char)average[n];
        }
    }
}

void Filter::equalizeExact(unsigned char* inPixels, unsigned char* outPixels, int ksize, const std::vector<int>& lightHitMap) {
    const int numElements = 3; //number of elements per pixel
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = initialPixel; i < numPixels; i++) {
        if ((width - offset <= i % width) && (i % width < offset)) continue;
        if (width * (height - offset) < i) continue;
        if (lightHitMap[i] < 64 || 192 <= lightHitMap[i]) {
            int average[numElements] = {};
            for (int n = 0; n < numElements; n++) {
                for (int ys = 0; ys < ksize; ys++) {
                    // Y pos on kernel
                    const int y = (ys - offset) * width * numElements;
                    for (int xs = 0; xs < ksize; xs++) {
                        // X pos on kernel
                        const int x = (xs - offset) * numElements;
                        average[n] += (int)inPixels[i * numElements + n + x + y];
                    }
                }
                average[n] = average[n] / (ksize * ksize);
            }
            //out
            for (int n = 0; n < numElements; n++) {
                outPixels[i * numElements + n] = (unsigned char)average[n];
            }
        }
        else {
            for (int n = 0; n < numElements; n++) {
                outPixels[i * numElements + n] = inPixels[i * numElements + n];
            }
        }
    }
}

void Filter::median(unsigned char* inPixels, unsigned char* outPixels, int ksize) {
    const int numElements = 3; //number of elements per pixel
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;
    const int medianPos = (ksize * ksize + 1) / 2;

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = initialPixel; i < numPixels; i++) {
        if ((width - offset <= i % width) && (i % width < offset)) continue;
        if (width * (height - offset) < i) continue;

        int outColor[numElements]  = { 0,0,0 };
        int median[numElements] = { 0,0,0 };

        for (int n = 0; n < numElements; n++) {
            std::vector<int> kernel;
            for (int ys = 0; ys < ksize; ys++) {
                // Y pos on kernel
                const int y = (ys - offset) * width * numElements;
                for (int xs = 0; xs < ksize; xs++) {
                    // X pos on kernel
                    const int x = (xs - offset) * numElements;
                    kernel.push_back(inPixels[i * numElements + n + x + y]);
                }
            }
            sort(kernel.begin(), kernel.end());
            outColor[n] = (unsigned char)kernel[medianPos];
        }

        //out
        for (int n = 0; n < numElements; n++) {
            outPixels[i * numElements + n] = (unsigned char)outColor[n];
        }
    }
}

void Filter::bilateral(unsigned char* inPixels, unsigned char* outPixels, int ksize, double sigma1, double sigma2, int iterate) { // ksize must be odd number
    const int numElements = 3; //number of elements per pixel
    const double sigmaTerm1 = 2.0 * sigma1 * sigma1;
    const double sigmaTerm2 = 2.0 * sigma2 * sigma2;
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;

    for (int d = 0; d < iterate; d++) {
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = initialPixel; i < numPixels; i++) {
            if ((width - offset <= i % width) && (i % width < offset)) continue;
            if (width * (height - offset) < i) continue;
            for (int n = 0; n < numElements; n++) {
                double cw = 0.0;
                double cwf = 0.0;
                for (int xs = -offset; xs < ksize - offset; xs++) {
                    // X pos on kernel
                    const int x = xs * numElements;
                    for (int ys = -offset; ys < ksize - offset; ys++) {
                        // Y pos on kernel
                        const int y = ys * width * numElements;
                        const double f0 = (double)inPixels[i * numElements + n];
                        const double f1 = (double)inPixels[i * numElements + n + x + y];
                        const double f0_f1 = f0 - f1;
                        const double g1 = exp(-((xs * xs + ys * ys) / sigmaTerm1));
                        const double g2 = exp(-((f0_f1 * f0_f1) / sigmaTerm2));
                        const double w = g1 * g2;
                        const double wf = w * f1;
                        cw = cw + w;
                        cwf = cwf + wf;
                    }
                }
                outPixels[i * numElements + n] = (unsigned char)(cwf / cw);
            }
        }
        for (int i = 0; i < numPixels; i++) {
            inPixels[i * 3] = outPixels[i * 3];
            inPixels[i * 3 + 1] = outPixels[i * 3 + 1];
            inPixels[i * 3 + 2] = outPixels[i * 3 + 2];
        }
    }
}

void Filter::jointBilateral(unsigned char* inPixels, unsigned char* outPixels, int ksize, double sigma1, double sigma2, int iterate, Map* map) { // ksize must be odd number
    const int numElements = 3; //number of elements per pixel
    const double sigmaTerm1 = 2.0 * sigma1 * sigma1;
    const double sigmaTerm2 = 2.0 * sigma2 * sigma2;
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;
    
    // todo
    const double sigma3 = 1.0;

    for (int d = 0; d < iterate; d++) {
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = initialPixel; i < numPixels; i++) {
            if ((width - offset <= i % width) && (i % width < offset)) continue;
            if (width * (height - offset) < i) continue;
            for (int n = 0; n < numElements; n++) {
                double cw = 0.0;
                double cwf = 0.0;
                for (int xs = -offset; xs < ksize - offset; xs++) {
                    // X pos on kernel
                    const int x = xs * numElements;
                    for (int ys = -offset; ys < ksize - offset; ys++) {
                        // Y pos on kernel
                        const int y = ys * width * numElements;
                        // image buffer weight
                        const double f0 = (double)inPixels[i * numElements + n];
                        const double f1 = (double)inPixels[i * numElements + n + x + y];
                        const double f0_f1 = f0 - f1;
                        const double g1 = exp(-((xs * xs + ys * ys) / sigmaTerm1));
                        const double g2 = exp(-((f0_f1 * f0_f1) / sigmaTerm2));
                        const double wbf = g1 * g2;
                        // auxiliary buffers weight
                        double w_aux = 0.0;
                        for (int m = 0; m < map->getNumAuxMaps(); m++) {
                            const double fip = (double)map->getAuxMaps(m)[i * numElements + n];
                            const double fiq = (double)map->getAuxMaps(m)[i * numElements + n + x + y];
                            const double fip_fiq = fip - fiq;
                            w_aux += exp(-(fip_fiq * fip_fiq) / (2.0 * sigma3 * sigma3));
                        }
                        // total weight
                        const double w = wbf * w_aux;
                        const double wf = w * f1;
                        cw = cw + w;
                        cwf = cwf + wf;
                    }
                }
                outPixels[i * numElements + n] = (unsigned char)(cwf / cw);
            }
        }
        for (int i = 0; i < numPixels; i++) {
            inPixels[i * 3] = outPixels[i * 3];
            inPixels[i * 3 + 1] = outPixels[i * 3 + 1];
            inPixels[i * 3 + 2] = outPixels[i * 3 + 2];
        }
    }
}

void Filter::gaussian(unsigned char* inPixels, unsigned char* outPixels, int ksize, double sigma) {
    const int numElements = 3; //number of elements per pixel
    const double sigmaTerm = 2.0 * sigma * sigma;
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;

#pragma omp parallel for schedule(dynamic, 1)
    for (int i = initialPixel; i < numPixels; i++) {
        if ((width - offset <= i % width) && (i % width < offset)) continue;
        if (width * (height - offset) < i) continue;
        for (int n = 0; n < numElements; n++) {
            double cw = 0.0;
            double cwf = 0.0;
            for (int xs = -offset; xs < ksize - offset; xs++) {
                // X pos on kernel
                const int x = xs * numElements;
                for (int ys = -offset; ys < ksize - offset; ys++) {
                    // Y pos on kernel
                    const int y = ys * width * numElements;
                    const double f0 = (double)inPixels[i * numElements + n];
                    const double f1 = (double)inPixels[i * numElements + n + x + y];
                    const double g = exp(-((xs * xs + ys * ys) / sigmaTerm));
                    const double w = (1.0 / (sigmaTerm * M_PI)) * g;
                    const double wf = w * f1;
                    cw = cw + w;
                    cwf = cwf + wf;
                }
            }
            outPixels[i * numElements + n] = (unsigned char)(cwf / cw);
        }
    }
}

void Filter::bilateralWithLHWeight(unsigned char* inPixels, unsigned char* outPixels, const std::vector<int>& lightHitMap, int ksize, double sigma1, double sigma2, int iterate) { // ksize must be odd number
    const int numElements = 3; //number of elements per pixel
    const double sigmaTerm1 = 2.0 * sigma1 * sigma1;
    const double sigmaTerm2 = 2.0 * sigma2 * sigma2;
    const int offset = (ksize - 1) / 2;
    const int initialPixel = offset * width + offset;

    for (int d = 0; d < iterate; d++) {
#pragma omp parallel for schedule(dynamic, 1)
        for (int i = initialPixel; i < numPixels; i++) {
            if ((width - offset <= i % width) && (i % width < offset)) continue;
            if (width * (height - offset) < i) continue;
            // calculate kernel weight
            int kareaSum = 0;
            for (int ys = 0; ys < ksize; ys++) {
                // Y pos on kernel
                const int y = (ys - offset) * width;
                for (int xs = 0; xs < ksize; xs++) {
                    // X pos on kernel
                    const int x = xs - offset;
                    kareaSum += (int)lightHitMap[i + x + y];
                }
            }

            for (int n = 0; n < numElements; n++) {
                double cw = 0.0;
                double cwf = 0.0;
                for (int xs = -offset; xs < ksize - offset; xs++) {
                    // X pos on kernel
                    const int x = xs * numElements;
                    for (int ys = -offset; ys < ksize - offset; ys++) {
                        // Y pos on kernel
                        const int y = ys * width * numElements;
                        const double f0 = (double)inPixels[i * numElements + n];
                        const double f1 = (double)inPixels[i * numElements + n + x + y];
                        const double f0_f1 = f0 - f1;
                        const double g1 = exp(-((xs * xs + ys * ys) / sigmaTerm1));
                        const double g2 = exp(-((f0_f1 * f0_f1) / sigmaTerm2));
                        const double w = g1 * g2;
                        const double wf = w * f1;
                        cw += w * ((double)lightHitMap[i + x + y] / (double)kareaSum);
                        cwf += wf;
                    }
                }
                outPixels[i * numElements + n] = (unsigned char)(cwf / cw);
            }
        }
        for (int i = 0; i < numPixels; i++) {
            inPixels[i * 3] = outPixels[i * 3];
            inPixels[i * 3 + 1] = outPixels[i * 3 + 1];
            inPixels[i * 3 + 2] = outPixels[i * 3 + 2];
        }
    }
}

void Filter::writeImage(unsigned char* outPixels, const char *_name) {
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
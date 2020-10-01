#pragma once

namespace hiraishi {
    class Filter {
    private:
        int numPixels;
        int width;
        int height;

    public:
        Filter() {}
        ~Filter() {}

        void init(const int _width, const int _height, const int _numPixels);
        void grayscale(unsigned char* inPixels, unsigned char* outPixels);
        void equalize(unsigned char* inPixels, unsigned char* outPixels, int ksize);
        void equalizeWithLHWeight(unsigned char* inPixels, unsigned char* outPixels, int ksize, const std::vector<int>& lightHitMap);
        void equalizeExact(unsigned char* inPixels, unsigned char* outPixels, int ksize, const std::vector<int>& lightHitMap);
        void median(unsigned char* inPixels, unsigned char* outPixels, int ksize);
        void bilateral(unsigned char* inPixels, unsigned char* outPixels, int ksize, double sigma1, double sigma2, int iterate);
        void jointBilateral(unsigned char* inPixels, unsigned char* outPixels, int size, double sigma1, double sigma2, int iterate, Map* map);
        void gaussian(unsigned char* inPixels, unsigned char* outPixels, int ksize, double sigma);
        void bilateralWithLHWeight(unsigned char* inPixels, unsigned char* outPixels, const std::vector<int>& lightHitMap, int ksize, double sigma1, double sigma2, int iterate);
        void writeImage(unsigned char* outPixels, const char *_name);
    };
}
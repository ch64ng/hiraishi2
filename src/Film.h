#pragma once

namespace hiraishi {
    class Film {
    private:
        unsigned char *pixels;
        int numPixels;
        long long msec;

    public:
        Film() {}
        ~Film() {}

        int width = 200;
        int height = 200;
        int spp;

        unsigned char* getPixels() const { return pixels; }
        unsigned char getPixel(const int p) const { return pixels[p]; }
        int getNumPixels() const { return numPixels; }

        void init();
        void setPixelColor(const Vec3& c, int p);
        void setRenderStatus(const long long _msec, const int _spp);
        void writeImage();
        void writePixels();
        void writePixels1(std::string filename, int offset);
    };
}
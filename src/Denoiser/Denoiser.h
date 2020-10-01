#pragma once

namespace hiraishi {
    class Denoiser {
    private:
        unsigned char *bufPixels;
        unsigned char *outPixels;
        int numPixels;
        int width;
        int height;
        Filter filter;

    public:
        Denoiser() {}
        ~Denoiser() {}

        void init(Film* film);
        void denoise(Film* film, Map* map);
        void writeImage(const char *_name);
    };
}
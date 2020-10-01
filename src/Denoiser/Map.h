#pragma once

#include "../Vec3.h"

namespace hiraishi{
    class Map {
    private:
        const int numAuxMaps = 4;
        unsigned char* auxMaps[4];
        // auxMaps index
        // 0 : normal map
        // 1 : depth map
        // 2 : visibility map
        // 3 : albedo map
        std::vector<int> lightHitMap;
        // num hit to light
        int width;
        int height;
        int numPixels;
        int spp;
        double maxDepth = 1.7;
        double minDepth = -1.1;
        double depthRange = maxDepth - minDepth;

    public:
        Map() {}
        ~Map() {}

        void init(const int _width, const int _height, const int _spp);
        void setNormal(const Vec3& n, const int p);
        void setDepth(const double d, const int p);
        void setVisibility(const int p);
        void setAlbedo(const Vec3& a, const int p);
        void setLightHit(const int p);
        void normalizeLH();
        void optimizeLH();
        int getNumAuxMaps() const { return numAuxMaps; }
        unsigned char* getAuxMaps(const int m) const { return auxMaps[m]; }
        const std::vector<int>& getLightHitMap() const { return lightHitMap; }
        void writeImage();
    };
}
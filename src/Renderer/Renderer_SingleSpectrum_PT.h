#pragma once

namespace hiraishi {
    class Renderer_SingleSpectrum_PT : public Renderer {
    private:
        ModelSet model;

        void renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng);
        void initSpectrum();
        Spectrum RGB2Spectrum(const Vec3 rgb, int type);
        Vec3 Spectrum2RGB(const Spectrum& s);
        double averageSpectrumSamples(const double* lambda, const double* vals, int n, double lambdaStart, double lambdaEnd);

    private:
        Spectrum X, Y, Z;
        Spectrum rgbRefl2SpectWhite,
            rgbRefl2SpectCyan,
            rgbRefl2SpectMagenta,
            rgbRefl2SpectYellow,
            rgbRefl2SpectRed,
            rgbRefl2SpectGreen,
            rgbRefl2SpectBlue;
        Spectrum rgbIllum2SpectWhite,
            rgbIllum2SpectCyan,
            rgbIllum2SpectMagenta,
            rgbIllum2SpectYellow,
            rgbIllum2SpectRed,
            rgbIllum2SpectGreen,
            rgbIllum2SpectBlue;

        Spectrum light;

    public:
        Renderer_SingleSpectrum_PT() {}
        Renderer_SingleSpectrum_PT(const ModelSet& model_) {
            model = model_;
        }
        ~Renderer_SingleSpectrum_PT() {}

        void render(const Scene* scene, const Camera* camera, Film* film);
    };
}
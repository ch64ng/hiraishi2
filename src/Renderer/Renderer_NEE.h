#pragma once

namespace hiraishi {
    class Renderer_NEE : public Renderer {
    private:
        ModelSet model;

        void renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng);

    public:
        Renderer_NEE() {}
        Renderer_NEE(const ModelSet& model_) {
            model = model_;
        }
        ~Renderer_NEE() {}

        void render(const Scene* scene, const Camera* camera, Film* film);
    };
}
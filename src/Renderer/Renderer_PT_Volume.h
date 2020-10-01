#pragma once

namespace hiraishi {
    class Renderer_PT_Volume : public Renderer {
    private:
        ModelSet model;

        void renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng);

    public:
        Renderer_PT_Volume() {}
        Renderer_PT_Volume(const ModelSet& model_) {
            model = model_;
        }
        ~Renderer_PT_Volume() {}

        void render(const Scene* scene, const Camera* camera, Film* film);
    };
}
#pragma once

namespace hiraishi {
    class Renderer_PT : public Renderer {
    private:
        ModelSet model;

        void renderPixel(const Scene* scene, const Camera* camera, Film* film, const int i, const int j, Random& rng);

    public:
        Renderer_PT() {}
        Renderer_PT(const ModelSet& model_) {
            model = model_;
        }
        ~Renderer_PT() {}

        void render(const Scene* scene, const Camera* camera, Film* film);
    };
}
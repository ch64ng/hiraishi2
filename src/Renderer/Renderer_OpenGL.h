#pragma once

namespace hiraishi {
    class Renderer_OpenGL {
    private:
        ModelSet model;

    public:
        Renderer_OpenGL() {}
        ~Renderer_OpenGL() {}

        void render(const Scene* scene, const Camera* camera, Film* film);
    };
}
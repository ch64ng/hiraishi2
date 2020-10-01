#pragma once

namespace hiraishi {
    class Renderer {
    private:
    public:
        Renderer() {}
        ~Renderer() {}

        int spp = 1;
        int maxBounce = 15;
    };
}
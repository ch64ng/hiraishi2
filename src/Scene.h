#pragma once

namespace hiraishi {
    class Scene {
    private:
        ModelSet model;

    public:
        Scene() {}
        ~Scene() {}

        std::string objPath;
        std::string mtlPath;
        double scale = 1.0;

        void setModel(const ModelSet& modelset) { model = modelset; }
        const ModelSet& getModel() const { return model; }

        void init(const int w, const int h);
        Intersect intersect(const Ray& ray, Random& rng) const;
        void setVColor(const Vec3& color, const int vi) { model.setVColor(color, vi); }
    };
}
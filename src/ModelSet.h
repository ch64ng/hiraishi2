#pragma once

namespace hiraishi {
    class ModelSet {
    private:
        std::vector<Vec3> vertices;
        std::vector<Vec3> vNormals;
        std::vector<Vec3> texCoords;
        std::vector<Vec3> vColors;
        std::vector<Material> materials;
        std::vector<Face> faces;

        int lightIndex;
        double lightArea;

    public:
        ModelSet() {}
        virtual ~ModelSet() {}

        KdTree kdTree;

        void readMtl(const char *filename);
        void readObj(const char *filename);
        void printFaces();
        void makeFaceEquations();
        void initKdTree();
        void initVColor();
        
        void setVColor(const Vec3& color, const int vi) { vColors[vi] = color; }
        const std::vector<Vec3>& getVertices() const { return vertices; }
        const std::vector<Vec3>& getVNormals() const { return vNormals; }
        const std::vector<Vec3>& getVColors() const { return vColors; }
        const std::vector<Face>& getFaces() const { return faces; }
        const double& getLightArea() const { return lightArea; }
        Intersect intersect(const Ray& ray) const;
        Vec3 randomPosOnLight(Random& rng) const;
        void initLightArea();
        void addFace(Face f);
    };
}
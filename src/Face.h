#pragma once

namespace hiraishi {
    class Face {
    private:
        std::vector<long> vIndices;
        std::vector<long> vnIndices;
        std::vector<long> vtIndices;
        Vec3 normal;
        Vec3 pos;
        Vec3 midPos;
        BBox bbox;
        double r, g, b;
        Material *mtlPtr;

    public:
        Face() {}
        Face(const std::vector<int>& vi);
        ~Face() {}

        void makeEquation(const Vec3& p0, const Vec3& p1, const Vec3& p2);
        void appendVIndex(long v) {
            vIndices.push_back(v);
        }
        void appendVnIndex(long vn) {
            vnIndices.push_back(vn);
        }
        void appendVtIndex(long vt) {
            vtIndices.push_back(vt);
        }
        void clearVIndices() {
            vIndices.clear();
        }
        void setColor() {
            r = (double)rand() / (double)RAND_MAX;
            g = (double)rand() / (double)RAND_MAX;
            b = (double)rand() / (double)RAND_MAX;
        }
        void setMtlPtr(Material *m) { mtlPtr = m; }

        const size_t getNumVertices() const { return vIndices.size(); }
        const Vec3& getPos() const { return pos; }
        const Vec3& getMidPos() const { return midPos; }
        const BBox& getBBox() const { return bbox; }
        const Vec3& getNormal() const { return normal; }
        const long& getVIndex(const long i) const { return vIndices[i]; }
        const std::vector<long>& getVIndices() const { return vIndices; }
        const Material* getMtlPtr() const { return mtlPtr; }
        const double& getR() const { return r; }
        const double& getG() const { return g; }
        const double& getB() const { return b; }

        Face& operator=(const Face& f) {
            vIndices = f.vIndices;
            normal = f.normal;
            pos = f.pos;
            r = f.r;
            g = f.g;
            b = f.b;
            return *this;
        }

        bool intersect(const Vec3* v, const Ray& ray, double* t, Vec3& isectPos, Vec3& isectNormal, Material** mtlPtr) const;
        void print() const;
    };
}
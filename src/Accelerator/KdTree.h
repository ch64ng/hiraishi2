#pragma once

namespace hiraishi {
    struct Node {
        Node* children[2];
        BBox bbox;
        std::vector<Face*> faces;
    };

    class KdTree {
    private:
        std::vector<Face*> allFaces;

    public:
        Node* rootNode;

        void init(std::vector<Face>& faces);
        Node* build(const std::vector<Face*> &_faces, int depth);
        bool intersect(Node* node, const Ray& ray, Intersect& isect, const std::vector<Vec3>& vertices) const;
    };
}
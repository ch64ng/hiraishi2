#include <vector>
#include <algorithm>
#include "../Vec3.h"
#include "../Materials/Material.h"
#include "../Ray.h"
#include "../BBox.h"
#include "../Face.h"
#include "../Intersect.h"
#include "KdTree.h"

using namespace hiraishi;

void KdTree::init(std::vector<Face>& faces) {
    for (int i = 0; i < faces.size(); ++i) {
        allFaces.push_back(&faces[i]);
    }

    rootNode = build(allFaces, 0);
}

Node* KdTree::build(const std::vector<Face*> &_faces, int depth) {
    Node* node = new Node();
    node->faces = _faces;
    node->children[0] = NULL;
    node->children[1] = NULL;
    node->bbox = BBox();

    if (_faces.size() == 0) {
        return node;
    }

    if (_faces.size() == 1) {
        node->bbox = _faces[0]->getBBox();
        node->children[0] = new Node();
        node->children[1] = new Node();
        node->children[0]->faces = std::vector<Face*>();
        node->children[1]->faces = std::vector<Face*>();
        return node;
    }

    node->bbox = _faces[0]->getBBox();
    for (int i = 1; i < _faces.size(); ++i) {
        node->bbox.grow(_faces[i]->getBBox());
    }

    // get mid pos of all faces
    Vec3 midPos(0.0, 0.0, 0.0);
    for (int i = 0; i < _faces.size(); ++i) {
        midPos = midPos + (_faces[i]->getMidPos() * (1.0 / _faces.size()));
    }

    std::vector<Face*> leftFaces;
    std::vector<Face*> rightFaces;

    const int axis = depth % 3;
    for (int i = 0; i < _faces.size(); ++i) {
        switch (axis) {
            case 0: // axis = x
                if (midPos.x < _faces[i]->getMidPos().x)
                    leftFaces.push_back(_faces[i]);
                else
                    rightFaces.push_back(_faces[i]);
                break;
            case 1: // axis = y
                if (midPos.y < _faces[i]->getMidPos().y)
                    leftFaces.push_back(_faces[i]);
                else
                    rightFaces.push_back(_faces[i]);
                break;
            case 2: // axis = z
                if (midPos.z < _faces[i]->getMidPos().z)
                    leftFaces.push_back(_faces[i]);
                else
                    rightFaces.push_back(_faces[i]);
                break;
        }
    }

    if (leftFaces.size() == 0 && 0 < rightFaces.size())
        leftFaces = rightFaces;
    if (rightFaces.size() == 0 && 0 < leftFaces.size())
        rightFaces = leftFaces;

    int matches = 0;
    for (int i = 0; i < leftFaces.size(); ++i) {
        for (int j = 0; j < rightFaces.size(); ++j) {
            if (leftFaces[i] == rightFaces[j])
                matches++;
        }
    }

    if ((double)matches / leftFaces.size() < 0.5 && (double)matches / rightFaces.size() < 0.5) {
        node->children[0] = build(leftFaces, depth + 1);
        node->children[1] = build(rightFaces, depth + 1);
    }
    else {
        node->children[0] = new Node();
        node->children[1] = new Node();
        node->children[0]->faces = std::vector<Face*>();
        node->children[1]->faces = std::vector<Face*>();
    }

    return node;
}

bool KdTree::intersect(Node* node, const Ray& ray, Intersect& isect, const std::vector<Vec3>& vertices) const {
    bool hit = false;
    if (node == NULL)
        node = rootNode;
    if (node->bbox.intersect(ray)) {
        if (0 < node->children[0]->faces.size() || 0 < node->children[1]->faces.size()) {
            bool isectLeft = intersect(node->children[0], ray, isect, vertices);
            bool isectRight = intersect(node->children[1], ray, isect, vertices);
            return isectLeft || isectRight;
        }
        else {
            Material* isectMtl;
            for (int j = 0; j < node->faces.size(); ++j) {
                Vec3 isectPos, isectNormal;
                double t;
                Vec3 v[3];
                for (int i = 0; i < 3; i++) {
                    v[i] = vertices[node->faces[j]->getVIndex(i) - 1];
                }
                if (!node->faces[j]->intersect(v, ray, &t, isectPos, isectNormal, &isectMtl))
                    continue;
                if (t < isect.t) {
                    hit = true;
                    isect.t = t;
                    isect.mtlPtr = isectMtl;
                    isect.pos = isectPos;
                    isect.normal = isectNormal;
                }
            }
            return hit;
        }
    }
    return false;
}
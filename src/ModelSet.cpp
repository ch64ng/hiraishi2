#include <iostream>
#include <vector>
#include <sstream>
#include <assert.h>
#include <ctype.h>
#include "Random.h"
#include "Vec3.h"
#include "Sampler.h"
#include "Materials/Material.h"
#include "Ray.h"
#include "BBox.h"
#include "Face.h"
#include "Sphere.h"
#include "Intersect.h"
#include "Accelerator/KdTree.h"
#include "ModelSet.h"

using namespace hiraishi;

std::vector<std::string> split_naive(const std::string &s, char delim);
std::vector<std::string> getWords(char *str);

void ModelSet::readMtl(const char *filename) {
    FILE *fp;
    fopen_s(&fp, filename, "r");
    while (1) {
        char str[256];
        if (fgets(str, 256, fp) == NULL) break;
        std::vector<std::string> words = getWords(str);
        if (words.size() == 0) continue;
        if (words[0] == "newmtl") {
            std::vector<std::string> name = split_naive(words[1], '\n');
            materials.push_back(Material(name[0]));
        }
        else if (words[0] == "Ns") {
            const double ns = std::stof(words[1]);
            materials[materials.size() - 1].Ns = ns;
        }
        else if (words[0] == "Ni") {
            const double ni = std::stof(words[1]);
            materials[materials.size() - 1].Ni = ni;
        }
        else if (words[0] == "Tr") {
            const double tr = std::stof(words[1]);
            materials[materials.size() - 1].Tr = tr;
        }
        else if (words[0] == "d") {
            const double d = std::stof(words[1]);
            materials[materials.size() - 1].d = d;
        }
        else if (words[0] == "illum") {
            const int il = std::stoi(words[1]);
            materials[materials.size() - 1].illum = il;
        }
        else if (words[0] == "Ka") {
            const double r = std::stod(words[1]);
            const double g = std::stod(words[2]);
            const double b = std::stod(words[3]);
            materials[materials.size() - 1].Ka = Vec3(r, g, b);
        }
        else if (words[0] == "Kd") {
            const double r = std::stod(words[1]);
            const double g = std::stod(words[2]);
            const double b = std::stod(words[3]);
            materials[materials.size() - 1].Kd = Vec3(r, g, b);
        }
        else if (words[0] == "Ks") {
            const double r = std::stod(words[1]);
            const double g = std::stod(words[2]);
            const double b = std::stod(words[3]);
            materials[materials.size() - 1].Ks = Vec3(r, g, b);
        }
        else if (words[0] == "Ke") {
            const double r = std::stod(words[1]);
            const double g = std::stod(words[2]);
            const double b = std::stod(words[3]);
            materials[materials.size() - 1].Ke = Vec3(r, g, b);
        }
        else if (words[0] == "Tf") {
            const double r = std::stod(words[1]);
            const double g = std::stod(words[2]);
            const double b = std::stod(words[3]);
            materials[materials.size() - 1].Tf = Vec3(r, g, b);
        }
    }
    fclose(fp);
}

void ModelSet::readObj(const char *filename) {
    FILE *fp;
    fopen_s(&fp, filename, "r");

    int curMtlIndex = -1;
    int geoIndex = 0;
    while (1) {
        char str[256];
        if (fgets(str, 256, fp) == NULL) break;
        std::vector<std::string> words = getWords(str);
        if (words.size() == 0 || words[0] == "#") continue;
        if (str[0] == 'v') {
            Vec3 v;
            if (isblank(str[1])) {
                sscanf_s(str, "v %lf %lf %lf", &v.x, &v.y, &v.z);
                vertices.push_back(v);
            }
            else if (str[1] =='n') {
                sscanf_s(str, "vn %lf %lf %lf", &v.x, &v.y, &v.z);
                vNormals.push_back(v);
            }
            else if (str[1] == 't') {
                sscanf_s(str, "vt %lf %lf %lf", &v.x, &v.y, &v.z);
                texCoords.push_back(v);
            }
        }
        else if (str[0] == 'f') {
            Face f;
            if (words[1].find("/") != std::string::npos) {
                for (int i = 1; i <= 3; i++) {
                    std::vector<std::string> components = split_naive(words[i], '/');
                    f.appendVIndex(std::stoi(components[0]));
                    if (components.size() == 2) {
                        f.appendVtIndex(std::stoi(components[1]));
                    }
                    else if (components.size() == 3) {
                        f.appendVnIndex(std::stoi(components[1]));
                        f.appendVtIndex(std::stoi(components[2]));
                    }
                }
                f.setMtlPtr(&materials[curMtlIndex]);
                addFace(f);
            }
            else {
                char *sp = &str[1];
                while (*sp != '\n' && *sp != '\0') {
                    while (isblank(*sp)) sp++;
                    if (*sp == '\n' || *sp == '\0') break;
                    char word[256];
                    char *wp = word;
                    while (!isblank(*sp) && *sp != '\n' && *sp != '\0') {
                        *wp++ = *sp++;
                    }
                    *wp = 0;
                    if (atoi(word) < 0) {
                        const long v = vertices.size() + atoi(word) + 1;
                        f.appendVIndex((long)v);
                    }
                    else {
                        f.appendVIndex(atoi(word));
                    }
                }
                f.setMtlPtr(&materials[curMtlIndex]);
                addFace(f);
            }
        }
        else if (words[0] == "usemtl") {
            for (unsigned int i = 0; i < materials.size(); i++) {
                if (words[1] == materials[i].name) {
                    curMtlIndex = i;
                }
            }
        }
    }

    fclose(fp);
}

std::vector<std::string> getWords(char *str) {
    if (str[strlen(str) - 1] == '\n') str[strlen(str) - 1] = 0;
    int i = 0;
    while (iswspace(str[i])) i++;
    std::string line(&str[i]);
    return split_naive(line, ' ');
}

void ModelSet::printFaces() {
    for (unsigned int i = 0; i < vertices.size(); i++) {
        printf("v %f %f %f\n", vertices[i].x, vertices[i].y, vertices[i].z);
    }
}

void ModelSet::makeFaceEquations() {
    for (std::vector<Face>::iterator f = faces.begin(); f != faces.end(); f++) {
        std::vector<long> vi = f->getVIndices();
        f->makeEquation(vertices[vi[0] - 1], vertices[vi[1] - 1], vertices[vi[2] - 1]);
    }
}

void ModelSet::initKdTree() {
    std::cout << ">> kdTree : Generating" << std::endl;
    kdTree.init(faces);
    std::cout << ">> kdTree : FINISH" << std::endl << std::endl;
}

void ModelSet::initVColor() {
    for (int i = 0; i < vertices.size(); i++) {
        vColors.push_back(Vec3(0.75, 0.75, 0.75));
    }
}

std::vector<std::string> split_naive(const std::string &s, char delim) {
    std::vector<std::string> elems;
    std::string item;
    for (char ch : s) {
        if (ch == delim) {
            if (!item.empty())
                elems.push_back(item);
            item.clear();
        }
        else {
            item += ch;
        }
    }
    if (!item.empty())
        elems.push_back(item);
    return elems;
}

Intersect ModelSet::intersect(const Ray& ray) const {
    Intersect isect;
#if 1 // Use Kd-Tree
    Node* node = NULL;
    bool isIsect = kdTree.intersect(node, ray, isect, vertices);
#else 
    Material* isectMtl;
    for (std::vector<Face>::const_iterator f = faces.begin(); f != faces.end(); f++) {
        Vec3 isectPos, isectNormal;
        double t;
        Vec3 v[3];
        for (int i = 0; i < 3; i++) {
            v[i] = vertices[f->getVIndex(i) - 1];
        }
        if (!f->intersect(v, ray, &t, isectPos, isectNormal, &isectMtl)) continue;
        if (t < isect.t) {
            isect.t = t;
            isect.mtlPtr = isectMtl;
            isect.pos = isectPos;
            isect.normal = isectNormal;
        }
    }
#endif
    return isect;
}

Vec3 ModelSet::randomPosOnLight(Random& rng) const {
    //const int r = rng.intNext(0, geometries[lightIndex].getNumFaces() - 1);
    //const Face f = geometries[lightIndex].getFace(r);
    Vec3 v[3];
    for (int i = 0; i < 3; i++) {
        //v[i] = vertices[f.getVIndex(i) - 1];
    }
    Vec3 answer = Sampler::uniformSampleTriangle(v, rng.next(), rng.next());
    return answer;
}

void ModelSet::initLightArea() {
    //double answer = 0.0;
    //for (int i = 0; i < geometries[lightIndex].getNumFaces(); i++) {
    //    const Face f = geometries[lightIndex].getFace(i);
    //    Vec3 v[3];
    //    for (int j = 0; j < 3; j++) {
    //        v[j] = vertices[f.getVIndex(j) - 1];
    //    }
    //    // calculate area per face
    //    const Vec3 va1 = v[1] - v[0];
    //    const Vec3 va2 = v[2] - v[0];
    //    const double dot = Vec3::dot(va1, va2);
    //    const double area = sqrt(va1.length2() * va2.length2() - dot * dot);
    //    answer += area;
    //}
    //lightArea = answer;
}

void ModelSet::addFace(Face f) {
    faces.push_back(f);
}
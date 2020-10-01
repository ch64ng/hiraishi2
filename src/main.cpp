#include <iostream>
#include <string>
#include <vector>
#include <ctype.h>
#include <GL/glut.h>
#include "Vec3.h"
#include "Spectrum.h"
#include "Denoiser/Map.h"
#include "Sampler.h"
#include "Camera.h"
#include "Materials/Material.h"
#include "Ray.h"
#include "BBox.h"
#include "Face.h"
#include "Sphere.h"
#include "Intersect.h"
#include "Accelerator/KdTree.h"
#include "ModelSet.h"
#include "Film.h"
#include "Scene.h"
#include "Renderer/Renderer.h"
#include "Renderer/Renderer_PT.h"
#include "Renderer/Renderer_NEE.h"
#include "Renderer/Renderer_SingleSpectrum_PT.h"
#include "Renderer/Renderer_PT_Volume.h"
#include "Renderer/Renderer_OpenGL.h"
#include "Denoiser/Filter.h"
#include "Denoiser/Denoiser.h"

using namespace hiraishi;

std::vector<std::string> getWords(char *str);
Scene scene;
Camera camera;
Renderer_PT renderer;
Renderer_OpenGL GLRenderer;
Film film;
Map map;
Denoiser denoiser;

bool isGLDraw = true;

void printHelp() {
    std::cout
        << "---hiraishi---"                     << std::endl
        << std::endl
        << "------------------------------"     << std::endl
        << std::endl
        << "Key Config"                         << std::endl
        << std::endl
        << "[ R ] : Start Rendering"            << std::endl
        << "[ O ] : Output Rendered Image"      << std::endl
        << "[ Q ] : Toggle OpenGL"              << std::endl
        << std::endl
        << "[ W ] : Camera : Move Z-"           << std::endl
        << "[ S ] : Camera : Move Z+"           << std::endl
        << "[ A ] : Camera : Move X-"           << std::endl
        << "[ D ] : Camera : Move X+"           << std::endl
        << "[ SPACE ] : Camera : Move Y+"       << std::endl
        << "[ TAB ] : Camera : Move Y-"         << std::endl
        << std::endl
        << "Note : DO NOT start rendering before FINISH to generate Kd-Tree." << std::endl
        << std::endl
        << "------------------------------"     << std::endl
        << std::endl;
}

void initialReadPreferences(const char *filename) {
    FILE *fp;
    fopen_s(&fp, filename, "r");

    while (1) {
        char str[256];
        if (fgets(str, 256, fp) == NULL) break;
        std::vector<std::string> words = getWords(str);
        if (words.size() == 0 || words[0] == "#") continue;
        if (words[0] == "Renderer.spp") renderer.spp = atoi(words[1].c_str());
        if (words[0] == "Renderer.maxBounce") renderer.maxBounce = atoi(words[1].c_str());
        if (words[0] == "Film.width") film.width = atoi(words[1].c_str());
        if (words[0] == "Film.height") film.height = atoi(words[1].c_str());
        if (words[0] == "Scene.obj") scene.objPath = words[1];
        if (words[0] == "Scene.mtl") scene.mtlPath = words[1];
        if (words[0] == "Scene.scale") scene.scale = atof(words[1].c_str());
        if (words[0] == "Camera.eye") camera.setEye(Vec3(atof(words[1].c_str()), atof(words[2].c_str()), atof(words[3].c_str())));
        if (words[0] == "Camera.center") camera.setCenter(Vec3(atof(words[1].c_str()), atof(words[2].c_str()), atof(words[3].c_str())));
        if (words[0] == "Camera.fov") camera.setFovDeg(atof(words[1].c_str()));
    }

    fclose(fp);
}

void readPreferences(const char* filename) {
    FILE* fp;
    fopen_s(&fp, filename, "r");

    while (1) {
        char str[256];
        if (fgets(str, 256, fp) == NULL) break;
        std::vector<std::string> words = getWords(str);
        if (words.size() == 0 || words[0] == "#") continue;
        if (words[0] == "Renderer.spp") renderer.spp = atoi(words[1].c_str());
        if (words[0] == "Scene.scale") scene.scale = atof(words[1].c_str());
        if (words[0] == "Camera.eye") camera.setEye(Vec3(atof(words[1].c_str()), atof(words[2].c_str()), atof(words[3].c_str())));
        if (words[0] == "Camera.center") camera.setCenter(Vec3(atof(words[1].c_str()), atof(words[2].c_str()), atof(words[3].c_str())));
        if (words[0] == "Camera.fov") camera.setFovDeg(atof(words[1].c_str()));
    }

    fclose(fp);
}

void checkError(char* label) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf("%s: %s\n", label, gluErrorString(error));
    }
}

void draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isGLDraw) {
        GLRenderer.render(&scene, &camera, &film);
    }
    else {
        glDrawPixels(film.width, film.height, GL_RGB, GL_UNSIGNED_BYTE, (const GLvoid*)film.getPixels());
    }

    checkError("draw");
    glFlush();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, (double)width / (double)height, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyFunc(unsigned char key, int x, int y) {
    switch (key) {
    case 'q':
    case 'Q':
        isGLDraw = true;
        break;
    case 'r':
    case 'R':
        isGLDraw = false;
        readPreferences("preferences.txt");
        camera.init(film.width, film.height);
        renderer.render(&scene, &camera, &film);
        break;
    case 'o':
    case 'O':
        film.writeImage();
        break;
    case 'w':
    case 'W':
        camera.setEye(camera.getEye() + Vec3(0.0, 0.0, -0.1));
        break;
    case 's':
    case 'S':
        camera.setEye(camera.getEye() + Vec3(0.0, 0.0, 0.1));
        break;
    case 'a':
    case 'A':
        camera.setEye(camera.getEye() + Vec3(-0.1, 0.0, 0.0));
        break;
    case 'd':
    case 'D':
        camera.setEye(camera.getEye() + Vec3(0.1, 0.0, 0.0));
        break;
    case VK_SPACE:
        camera.setEye(camera.getEye() + Vec3(0.0, 0.1, 0.0));
        break;
    case VK_TAB:
        camera.setEye(camera.getEye() + Vec3(0.0, -0.1, 0.0));
        break;
    }
    glutPostRedisplay();
}

void refresh() {
    if (isGLDraw) return;
    glutPostRedisplay();
}

void initHiraishi() {
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.7, 0.9, 1.0, 1.0);

    // init objects
    scene.init(film.width, film.height);
    camera.init(film.width, film.height);
    film.init();
    map.init(film.width, film.height, renderer.spp);
}

void main(int argc, char** argv) {
    printHelp();

    // read settings
    initialReadPreferences("preferences.txt");

    // glut
    glutInit(&argc, argv);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(film.width, film.height);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
    glutCreateWindow("hiraishi");
    
    // init funcs
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyFunc);
    glutIdleFunc(refresh);

    // init
    initHiraishi();

    //renderer.render(&scene, &camera, &film);

    // main loop
    glutMainLoop();
}
#include <GL/glut.h>
#include "../Vec3.h"
#include "../Sampler.h"
#include "../Camera.h"
#include "../Materials/Material.h"
#include "../BBox.h"
#include "../Face.h"
#include "../Intersect.h"
#include "../Accelerator/KdTree.h"
#include "../ModelSet.h"
#include "../Film.h"
#include "../Scene.h"
#include "Renderer_OpenGL.h"

using namespace hiraishi;

void Renderer_OpenGL::render(const Scene* scene, const Camera* camera, Film* film) {
    model = scene->getModel();

    const Vec3 eye = camera->getEye();
    const Vec3 center = camera->getCenter();

    glLoadIdentity();

    glPushMatrix();
    gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, 0.0, 1.0, 0.0);
    glBegin(GL_TRIANGLES);

    for (unsigned int i = 0; i < model.getFaces().size(); ++i) {
        const Face face = model.getFaces()[i];
        const Vec3 color = face.getMtlPtr()->Kd;
        for (int j = 0; j < 3; ++j) {
            const Vec3 v = model.getVertices()[face.getVIndex(j) - 1];
            const Vec3 vColor = model.getVColors()[face.getVIndex(j) - 1];
            glColor3d(vColor.x, vColor.y, vColor.z);
            glVertex3f(v.x, v.y, v.z);
        }
    }

    glEnd();
    glPopMatrix();
}

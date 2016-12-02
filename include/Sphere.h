////////////////////////////////////////////////////////////////
// Sphere.h
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#ifndef SPHERE_H
#define SPHERE_H

#ifdef __APPLE__
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/glext.h>
#pragma comment(lib, "glew32.lib")
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

class Sphere {
public:
    Sphere(float radius, int slices, vec4 color);
    virtual ~Sphere()

    createVao(GLint _vao, GLint _program);

    vec4 *points;
    vec4 *colors;
    vec4 *normals;
    int *indices;
    int numVertices;

    void draw();
};

#endif // SPHERE_H

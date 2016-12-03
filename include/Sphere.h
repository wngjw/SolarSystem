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

#include <cmath>

using namespace glm;
using namespace std;

const float PI = 3.14159265;

class Sphere {
public:
    Sphere(float radius = 1.0, int slices = 30, vec4 color = vec4(0,0,0,1));
    virtual ~Sphere();

    GLint vao;
    void createVao(GLint _vao, GLint _program);

    vec4 *points;
    vec4 *colors;
    vec4 *normals;
    int *indices;
    int numVertices;

    void draw();
};

#endif // SPHERE_H

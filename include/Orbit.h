////////////////////////////////////////////////////////////////
// Orbit.h
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#ifndef ORBIT_H
#define ORBIT_H

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

using glm::vec4;

class Orbit
{
public:
    Orbit(float majorAxis = 1.0, float minorAxis = 1.0, int numPoints = 30);
    virtual ~Orbit();

    GLint vao;
    void createVAO(GLint _vao, GLint _program);
    vec4 *points;
    vec4 *colors;
    vec4 *normals;
    int numVertices;
    void draw();
};

#endif // ORBIT_H

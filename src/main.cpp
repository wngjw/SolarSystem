////////////////////////////////////////////////////////////////
// main.cpp
// For the final project of fall 2016 Computer Graphics
//  with Dr. Yerion
//
// This will draw our solar system with the sun generating light
//  the planets and their rings, an approximated asteroid belt
//  and a comet with a tail and a large flag.
//
// TODO : CONTROLS
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>
#include <fstream>

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
// end paste
#include "../include/shader.h"
#include "../include/Sphere.h"

using namespace glm;
using namespace std;

Sphere sun;

static mat4 modelViewMat = mat4(1.0);
static mat4 projMat = mat4(1.0);
static mat3 normalMat = mat3(1.0);

static const vec4 globAmb = vec4(0.2, 0.2, 0.2, 1.0);

static unsigned int vertexShaderId, fragmentShaderId, programId, sunVao, modelViewMatLoc, normalMatLoc, projMatLoc;

void setup()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    vertexShaderId = setShader("vertex", "vshader53.glsl");
    fragmentShaderId = setShader("fragment", "fshader53.glsl");
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glUseProgram(programId);

    modelViewMatLoc = glGetUniformLocation(programId,"modelViewMat");
    projMatLoc = glGetUniformLocation(programId,"projMat");
    normalMatLoc = glGetUniformLocation(programId,"normalMat");

    sun = Sphere(5.0, 30, vec4(1, 1, 0, 1));
    sun.createVao(sunVao, programId);
}

void drawScene()
{
    projMat = frustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
    glUniformMatrix4fv(projMatLoc, 1, GL_FALSE, value_ptr(projMat));

    //modelViewMat = mat4(1.0);
    modelViewMat = lookAt(vec3(0.0, 0.0, 10.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0));
    glUniformMatrix4fv(modelViewMatLoc, 1, GL_FALSE, value_ptr(modelViewMat));
    normalMat = transpose(inverse(mat3(modelViewMat)));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, value_ptr(normalMat));

    cout << "drawing" << endl;
    sun.draw();
    cout << "end draw" << endl;
}

void keyInput(unsigned char key, int x, int y)
{
    switch(key)
    {
    case 27:
        exit(0);
        break;
    default:
        break;
    }
}

void resize(int w, int h)
{
    glViewport(0, 0, w, h);
}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );

    glutInitContextVersion(4, 3);
    glutInitContextProfile(GLUT_CORE_PROFILE);
    glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    glutInitWindowSize(800, 450);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Solar System");
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyInput);

    glewExperimental = GL_TRUE;
    glewInit();

    setup();

    glutMainLoop();
}

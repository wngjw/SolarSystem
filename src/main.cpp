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

static unsigned int vertexShaderId, fragmentShaderId, programId, sunVao;

void setup()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glEnable(GL_DEPTH_TEST);

    // Create shader program executable.

    vertexShaderId = setShader("vertex", "vshader53.glsl");

    fragmentShaderId = setShader("fragment", "fshader53.glsl");
    programId = glCreateProgram();
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);
    glLinkProgram(programId);
    glUseProgram(programId);

    sun = Sphere(5.0, 30, vec4(1, 1, 0, 1));
    sun.createVao(sunVao, programId);
}

void drawScene()
{
    cout << "drawing" << endl;
    sun.draw();
    cout << "end draw" << endl;
}

// Keyboard input processing routine.
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
    glutInitWindowSize(500, 500);
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

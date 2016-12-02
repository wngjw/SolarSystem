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

#include "../include/Orbit.h"

using namespace glm;

void setup()
{
}

void drawScene()
{

}

int main( int argc, char **argv )
{
    glutInit( &argc, argv );
    glutCreateWindow("Solar System");
    glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
    setup();
    glutDisplayFunc(drawScene);
    glutMainLoop();
    return 0;
}

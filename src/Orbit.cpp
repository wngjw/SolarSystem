////////////////////////////////////////////////////////////////
// Orbit.cpp
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////

#include "../include/Orbit.h"

using glm::vec4;

Orbit::Orbit(float majorAxis, float minorAxis, int numPoints)
{

}

Orbit::~Orbit()
{
    delete [] points;
    delete [] colors;
    delete [] normals;
}

void Orbit::createVAO(GLint _vao, GLint _program)
{
//    vao = _vao;
//    glBindVertexArray(vao);
//    GLuint buffer;
//    glGenBuffers(1, &buffer);
//    glBindBuffer(GL_ARRAY_BUFFER, buffer);
//
//    int csize = sizeof((*points)) * numVertices;
//    glBufferData(GL_ARRAY_BUFFER, 2*csize, NULL, GL_STATIC_DRAW);
//    glBufferSubData(GL_ARRAY_BUFFER, 0, csize, *points);
//    glBufferSubData(GL_ARRAY_BUFFER, csize, csize, *colors);
//
//    // set up shader variables
//    GLuint vPosition = glGetAttribLocation(_program, "vPosition");
//    glEnableVertexAttribArray(vPosition);
//    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
//    GLuint vColor = glGetAttribLocation(_program, "vColor");
//    glEnableVertexAttribArray(vColor);
//    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(csize));
//    glBindBuffer(GL_ARRAY_BUFFER, 0);
//    glBindVertexArray(0);
}

void Orbit::draw()
{

}

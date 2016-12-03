////////////////////////////////////////////////////////////////
// Sphere.cpp
//
// Tony Wang and Luke Johnson
////////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>

#include "../include/Sphere.h"

using namespace std;

Sphere::Sphere(float radius, int slices, vec4 color)
{
    numVertices = (slices + 1) * (slices + 1);

    points = new vec4[numVertices];
    colors = new vec4[numVertices];
    normals = new vec4[numVertices];
    indices = new int[numVertices];

    float theta, phi, x, y, z;
    int index = 0;
    vec4 center = vec4(0,0,0,1);

    int i = slices;
    int j = slices;

    for(int v = 0; v <= j; v++)
    {
        for(int u = 0; u <= i; u++)
        {
            theta = PI * ((float(u) / i) * (2.0 / slices));
            phi = PI * (1 + (float(v) / j) * (1.0 / slices));

            x = radius * cos(phi) * cos(theta);
            y = radius * sin(phi);
            z = radius * cos(phi) * sin(theta);

            points[index] = vec4(x,y,z,1.0);
            normals[index] = vec4(x,y,z,0.0);
            colors[index] = color;
            indices[index] = index;
            index++;
        }
    }
}


Sphere::~Sphere()
{
    delete[] points;
    delete[] colors;
    delete[] indices;
    delete[] indices;
}

void Sphere::createVao(GLint _vao, GLint program)
{
    vao = _vao;
    glBindVertexArray(vao);
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    int csize = sizeof((*points)) * numVertices;
    glBufferData(GL_ARRAY_BUFFER, 3 * csize, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, csize, points);
    glBufferSubData(GL_ARRAY_BUFFER, csize, csize, colors);
    glBufferSubData(GL_ARRAY_BUFFER, 2 * csize, csize, normals);

    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);

    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (void*)csize);

    GLuint vNormal = glGetAttribLocation(program, "vNormal");
    glEnableVertexAttribArray(vNormal);
    glVertexAttribPointer(vNormal, 4, GL_FLOAT, GL_FALSE, 0, (void*)(2 * csize));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void Sphere::draw() {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, numVertices);
    glBindVertexArray(0);
}
